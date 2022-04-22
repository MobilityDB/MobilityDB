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
 * @file tpoint_out.c
 * @brief Output of temporal points in WKT, EWKT, WKB, EWKB, and MF-JSON
 * format.
 */

#include "point/tpoint_out.h"

/* PostgreSQL */
#include <assert.h>
#include <float.h>
#include <utils/builtins.h>
/* PostGIS */
#if POSTGIS_VERSION_NUMBER >= 30000
#include <liblwgeom_internal.h>
#endif
/* MobilityDB */
#include "general/temporaltypes.h"
#include "general/tempcache.h"
#include "general/temporal_util.h"
#include "point/tpoint.h"
#include "point/tpoint_spatialfuncs.h"

/* The following definitions are taken from PostGIS */

#define OUT_SHOW_DIGS_DOUBLE 20
#define OUT_MAX_DOUBLE_PRECISION 15
#define OUT_MAX_DIGS_DOUBLE (OUT_SHOW_DIGS_DOUBLE + 2) /* +2 mean add dot and sign */
#if POSTGIS_VERSION_NUMBER < 30000
#define OUT_DOUBLE_BUFFER_SIZE \
  OUT_MAX_DIGS_DOUBLE + OUT_MAX_DOUBLE_PRECISION + 1
#endif

/*****************************************************************************
 * Output in WKT and EWKT format
 *****************************************************************************/

/**
 * Output a geometry in Well-Known Text (WKT) format.
 *
 * @note The parameter type is not needed for temporal points
 */
static char *
wkt_out(Oid typid __attribute__((unused)), Datum value)
{
  GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(value);
  LWGEOM *geom = lwgeom_from_gserialized(gs);
  size_t len;
  char *wkt = lwgeom_to_wkt(geom, WKT_ISO, DBL_DIG, &len);
  char *result = palloc(len);
  strcpy(result, wkt);
  lwgeom_free(geom);
  pfree(wkt);
  return result;
}

/**
 * Output a geometry in Extended Well-Known Text (EWKT) format,
 * that is, in WKT format prefixed with the SRID.
 *
 * @note The parameter type is not needed for temporal points
 */
char *
ewkt_out(Oid typid __attribute__((unused)), Datum value)
{
  GSERIALIZED *gs = (GSERIALIZED *)DatumGetPointer(value);
  LWGEOM *geom = lwgeom_from_gserialized(gs);
  size_t len;
  char *wkt = lwgeom_to_wkt(geom, WKT_EXTENDED, DBL_DIG, &len);
  char *result = palloc(len);
  strcpy(result, wkt);
  lwgeom_free(geom);
  pfree(wkt);
  return result;
}

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Output a temporal point in Well-Known Text (WKT) format.
 */
char *
tpoint_as_text(const Temporal *temp)
{
  char *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = tinstant_to_string((TInstant *) temp, &wkt_out);
  else if (temp->subtype == INSTANTSET)
    result = tinstantset_to_string((TInstantSet *) temp, &wkt_out);
  else if (temp->subtype == SEQUENCE)
    result = tsequence_to_string((TSequence *) temp, false, &wkt_out);
  else /* temp->subtype == SEQUENCESET */
    result = tsequenceset_to_string((TSequenceSet *) temp, &wkt_out);
  return result;
}

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Output a temporal point in Extended Well-Known Text (EWKT) format,
 * that is, in WKT format prefixed with the SRID.
 */
char *
tpoint_as_ewkt(const Temporal *temp)
{
  int srid = tpoint_srid(temp);
  char str1[20];
  if (srid > 0)
    sprintf(str1, "SRID=%d%c", srid,
      MOBDB_FLAGS_GET_LINEAR(temp->flags) ? ';' : ',');
  else
    str1[0] = '\0';
  char *str2 = tpoint_as_text(temp);
  char *result = (char *) palloc(strlen(str1) + strlen(str2) + 1);
  strcpy(result, str1);
  strcat(result, str2);
  pfree(str2);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Output a geometry/geography array in Well-Known Text (WKT) format
 */
char **
geoarr_as_text(const Datum *geoarr, int count, bool extended)
{
  char **result = palloc(sizeof(char *) * count);
  for (int i = 0; i < count; i++)
    /* The wkt_out and ewkt_out functions do not use the first argument */
    result[i] = extended ?
      ewkt_out(ANYOID, geoarr[i]) : wkt_out(ANYOID, geoarr[i]);
  return result;
}

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Output a temporal point array in Well-Known Text (WKT) or
 * Extended Well-Known Text (EWKT) format
 */
char **
tpointarr_as_text(const Temporal **temparr, int count, bool extended)
{
  char **result = palloc(sizeof(text *) * count);
  for (int i = 0; i < count; i++)
    result[i] = extended ? tpoint_as_ewkt(temparr[i]) :
      tpoint_as_text(temparr[i]);
  return result;
}

/*****************************************************************************
 * Output in MFJSON format
 *****************************************************************************/

/**
 * Return the maximum size in bytes of the coordinate array represented in
 * MF-JSON format
 */
static size_t
coordinates_mfjson_size(int npoints, bool hasz, int precision)
{
  assert(precision <= OUT_MAX_DOUBLE_PRECISION);
  if (hasz)
    return (OUT_MAX_DIGS_DOUBLE + precision + sizeof(","))
      * 3 * npoints + sizeof(",[]");
  else
    return (OUT_MAX_DIGS_DOUBLE + precision + sizeof(","))
      * 2 * npoints + sizeof(",[]");
}

/**
 * Writes into the buffer the coordinate array represented in MF-JSON format
 */
static size_t
coordinates_mfjson_buf(char *output, const TInstant *inst, int precision)
{
  char *ptr;
  char x[OUT_DOUBLE_BUFFER_SIZE];
  char y[OUT_DOUBLE_BUFFER_SIZE];

  assert (precision <= OUT_MAX_DOUBLE_PRECISION);
  ptr = output;

  if (MOBDB_FLAGS_GET_Z(inst->flags))
  {
    char z[OUT_DOUBLE_BUFFER_SIZE];
    const POINT3DZ *pt = datum_point3dz_p(tinstant_value(inst));
#if POSTGIS_VERSION_NUMBER < 30000
    lwprint_double(pt->x, precision, x, OUT_DOUBLE_BUFFER_SIZE);
    lwprint_double(pt->y, precision, y, OUT_DOUBLE_BUFFER_SIZE);
    lwprint_double(pt->z, precision, z, OUT_DOUBLE_BUFFER_SIZE);
#else
    lwprint_double(pt->x, precision, x);
    lwprint_double(pt->y, precision, y);
    lwprint_double(pt->z, precision, z);
#endif
    ptr += sprintf(ptr, "[%s,%s,%s]", x, y, z);
  }
  else
  {
    const POINT2D *pt = datum_point2d_p(tinstant_value(inst));
#if POSTGIS_VERSION_NUMBER < 30000
    lwprint_double(pt->x, precision, x, OUT_DOUBLE_BUFFER_SIZE);
    lwprint_double(pt->y, precision, y, OUT_DOUBLE_BUFFER_SIZE);
#else
    lwprint_double(pt->x, precision, x);
    lwprint_double(pt->y, precision, y);
#endif
    ptr += sprintf(ptr, "[%s,%s]", x, y);
  }
  return (ptr - output);
}

/**
 * Return the maximum size in bytes of the datetimes array represented
 * in MF-JSON format.
 *
 * For example `"datetimes":["2019-08-06T18:35:48.021455+02:30","2019-08-06T18:45:18.476983+02:30"]`
 * will return  2 enclosing brackets + 1 comma +
 * for each timestamptz 32 characters + 2 double quotes + 1 comma
 */
static size_t
datetimes_mfjson_size(int npoints)
{
  return sizeof("\"2019-08-06T18:35:48.021455+02:30\",") * npoints + sizeof("[],");
}

/**
 * Writes into the buffer the datetimes array represented in MF-JSON format
 */
static size_t
datetimes_mfjson_buf(char *output, const TInstant *inst)
{
  char *ptr = output;
  char *t = call_output(TIMESTAMPTZOID, TimestampTzGetDatum(inst->t));
  /* Replace ' ' by 'T' as separator between date and time parts */
  t[10] = 'T';
  ptr += sprintf(ptr, "\"%s\"", t);
  pfree(t);
  return (ptr - output);
}

/**
 * Return the maximum size in bytes of the SRS represented in MF-JSON format
 */
static size_t
srs_mfjson_size(char *srs)
{
  size_t size = sizeof("'crs':{'type':'name',");
  size += sizeof("'properties':{'name':''}},");
  size += strlen(srs) * sizeof(char);
  return size;
}

/**
 * Writes into the buffer the SRS represented in MF-JSON format
 */
static size_t
srs_mfjson_buf(char *output, char *srs)
{
  char *ptr = output;
  ptr += sprintf(ptr, "\"crs\":{\"type\":\"name\",");
  ptr += sprintf(ptr, "\"properties\":{\"name\":\"%s\"}},", srs);
  return (ptr - output);
}

/**
 * Return the maximum size in bytes of the bouding box represented in
 * MF-JSON format
 */
static size_t
bbox_mfjson_size(int hasz, int precision)
{
  /* The maximum size of a timestamptz is 35 characters, e.g., "2019-08-06 23:18:16.195062-09:30" */
  size_t size = sizeof("'stBoundedBy':{'period':{'begin':,'end':}},") +
    sizeof("\"2019-08-06T18:35:48.021455+02:30\",") * 2;
  if (! hasz)
  {
    size += sizeof("'bbox':[,,,],");
    size +=  2 * 2 * (OUT_MAX_DIGS_DOUBLE + precision);
  }
  else
  {
    size += sizeof("\"bbox\":[,,,,,],");
    size +=  2 * 3 * (OUT_MAX_DIGS_DOUBLE + precision);
  }
  return size;
}

/**
 * Writes into the buffer the bouding box represented in MF-JSON format
 */
static size_t
bbox_mfjson_buf(char *output, const STBOX *bbox, int hasz, int precision)
{
  char *ptr = output;
  ptr += sprintf(ptr, "\"stBoundedBy\":{");
  if (! hasz)
    ptr += sprintf(ptr, "\"bbox\":[%.*f,%.*f,%.*f,%.*f],",
      precision, bbox->xmin, precision, bbox->ymin,
      precision, bbox->xmax, precision, bbox->ymax);
  else
    ptr += sprintf(ptr, "\"bbox\":[%.*f,%.*f,%.*f,%.*f,%.*f,%.*f],",
      precision, bbox->xmin, precision, bbox->ymin, precision, bbox->zmin,
      precision, bbox->xmax, precision, bbox->ymax, precision, bbox->zmax);
  char *begin = call_output(TIMESTAMPTZOID, TimestampTzGetDatum(bbox->tmin));
  char *end = call_output(TIMESTAMPTZOID, TimestampTzGetDatum(bbox->tmax));
  ptr += sprintf(ptr, "\"period\":{\"begin\":\"%s\",\"end\":\"%s\"}},", begin, end);
  pfree(begin); pfree(end);
  return (ptr - output);
}

/*****************************************************************************/

/**
 * Return the maximum size in bytes of a temporal instant point represented
 * in MF-JSON format
 */
static size_t
tpointinst_as_mfjson_size(const TInstant *inst, int precision,
  const STBOX *bbox, char *srs)
{
  bool hasz = MOBDB_FLAGS_GET_Z(inst->flags);
  size_t size = coordinates_mfjson_size(1, hasz, precision);
  size += datetimes_mfjson_size(1);
  size += sizeof("{'type':'MovingPoint',");
  size += sizeof("'coordinates':,'datetimes':,'interpolations':['Discrete']}");
  if (srs) size += srs_mfjson_size(srs);
  if (bbox) size += bbox_mfjson_size(hasz, precision);
  return size;
}

/**
 * Writes into the buffer the temporal instant point represented in MF-JSON format
 */
static size_t
tpointinst_as_mfjson_buf(const TInstant *inst, int precision,
  const STBOX *bbox, char *srs, char *output)
{
  char *ptr = output;
  ptr += sprintf(ptr, "{\"type\":\"MovingPoint\",");
  if (srs) ptr += srs_mfjson_buf(ptr, srs);
  if (bbox) ptr += bbox_mfjson_buf(ptr, bbox, MOBDB_FLAGS_GET_Z(inst->flags),
    precision);
  ptr += sprintf(ptr, "\"coordinates\":");
  ptr += coordinates_mfjson_buf(ptr, inst, precision);
  ptr += sprintf(ptr, ",\"datetimes\":");
  ptr += datetimes_mfjson_buf(ptr, inst);
  ptr += sprintf(ptr, ",\"interpolations\":[\"Discrete\"]}");
  return (ptr - output);
}

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Return the temporal instant point represented in MF-JSON format
 */
char *
tpointinst_as_mfjson(const TInstant *inst, int precision,
  const STBOX *bbox, char *srs)
{
  size_t size = tpointinst_as_mfjson_size(inst, precision, bbox, srs);
  char *output = palloc(size);
  tpointinst_as_mfjson_buf(inst, precision, bbox, srs, output);
  return output;
}

/*****************************************************************************/

/**
 * Return the maximum size in bytes of a temporal instant set point
 * represented in MF-JSON format
 */
static size_t
tpointinstset_as_mfjson_size(const TInstantSet *ti, int precision, const STBOX *bbox,
  char *srs)
{
  bool hasz = MOBDB_FLAGS_GET_Z(ti->flags);
  size_t size = coordinates_mfjson_size(ti->count, hasz, precision);
  size += datetimes_mfjson_size(ti->count);
  size += sizeof("{'type':'MovingPoint',");
  size += sizeof("'coordinates':[],'datetimes':[],'interpolations':['Discrete']}");
  if (srs) size += srs_mfjson_size(srs);
  if (bbox) size += bbox_mfjson_size(hasz, precision);
  return size;
}

/**
 * Writes into the buffer the temporal instant set point represented in MF-JSON format
 */
static size_t
tpointinstset_as_mfjson_buf(const TInstantSet *ti, int precision, const STBOX *bbox,
  char *srs, char *output)
{
  char *ptr = output;
  ptr += sprintf(ptr, "{\"type\":\"MovingPoint\",");
  if (srs) ptr += srs_mfjson_buf(ptr, srs);
  if (bbox) ptr += bbox_mfjson_buf(ptr, bbox, MOBDB_FLAGS_GET_Z(ti->flags),
    precision);
  ptr += sprintf(ptr, "\"coordinates\":[");
  for (int i = 0; i < ti->count; i++)
  {
    if (i) ptr += sprintf(ptr, ",");
    ptr += coordinates_mfjson_buf(ptr, tinstantset_inst_n(ti, i), precision);
  }
  ptr += sprintf(ptr, "],\"datetimes\":[");
  for (int i = 0; i < ti->count; i++)
  {
    if (i) ptr += sprintf(ptr, ",");
    ptr += datetimes_mfjson_buf(ptr, tinstantset_inst_n(ti, i));
  }
  ptr += sprintf(ptr, "],\"interpolations\":[\"Discrete\"]}");
  return (ptr - output);
}

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Return the temporal instant set point represented in MF-JSON format
 */
char *
tpointinstset_as_mfjson(const TInstantSet *ti, int precision, const STBOX *bbox,
  char *srs)
{
  size_t size = tpointinstset_as_mfjson_size(ti, precision, bbox, srs);
  char *output = palloc(size);
  tpointinstset_as_mfjson_buf(ti, precision, bbox, srs, output);
  return output;
}

/*****************************************************************************/

/**
 * Return the maximum size in bytes of a temporal sequence point
 * represented in MF-JSON format
 */
static size_t
tpointseq_as_mfjson_size(const TSequence *seq, int precision,
  const STBOX *bbox, char *srs)
{
  bool hasz = MOBDB_FLAGS_GET_Z(seq->flags);
  size_t size = coordinates_mfjson_size(seq->count, hasz, precision);
  size += datetimes_mfjson_size(seq->count);
  size += sizeof("{'type':'MovingPoint',");
  /* We reserve space for the largest strings, i.e., 'false' and "Stepwise" */
  size += sizeof("'coordinates':[],'datetimes':[],'lower_inc':false,'upper_inc':false,interpolations':['Stepwise']}");
  if (srs) size += srs_mfjson_size(srs);
  if (bbox) size += bbox_mfjson_size(hasz, precision);
  return size;
}

/**
 * Writes into the buffer the temporal sequence point represented in MF-JSON format
 */
static size_t
tpointseq_as_mfjson_buf(const TSequence *seq, int precision, const STBOX *bbox,
  char *srs, char *output)
{
  char *ptr = output;
  ptr += sprintf(ptr, "{\"type\":\"MovingPoint\",");
  if (srs) ptr += srs_mfjson_buf(ptr, srs);
  if (bbox) ptr += bbox_mfjson_buf(ptr, bbox, MOBDB_FLAGS_GET_Z(seq->flags), precision);
  ptr += sprintf(ptr, "\"coordinates\":[");
  for (int i = 0; i < seq->count; i++)
  {
    if (i) ptr += sprintf(ptr, ",");
    ptr += coordinates_mfjson_buf(ptr, tsequence_inst_n(seq, i), precision);
  }
  ptr += sprintf(ptr, "],\"datetimes\":[");
  for (int i = 0; i < seq->count; i++)
  {
    if (i) ptr += sprintf(ptr, ",");
    ptr += datetimes_mfjson_buf(ptr, tsequence_inst_n(seq, i));
  }
  ptr += sprintf(ptr, "],\"lower_inc\":%s,\"upper_inc\":%s,\"interpolations\":[\"%s\"]}",
    seq->period.lower_inc ? "true" : "false", seq->period.upper_inc ? "true" : "false",
    MOBDB_FLAGS_GET_LINEAR(seq->flags) ? "Linear" : "Stepwise");
  return (ptr - output);
}

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Return the temporal sequence point represented in MF-JSON format
 */
char *
tpointseq_as_mfjson(const TSequence *seq, int precision, const STBOX *bbox,
  char *srs)
{
  size_t size = tpointseq_as_mfjson_size(seq, precision, bbox, srs);
  char *output = palloc(size);
  tpointseq_as_mfjson_buf(seq, precision, bbox, srs, output);
  return output;
}

/*****************************************************************************/

/**
 * Return the maximum size in bytes of a temporal sequence set point
 * represented in MF-JSON format
 */
static size_t
tpointseqset_as_mfjson_size(const TSequenceSet *ts, int precision, const STBOX *bbox,
  char *srs)
{
  bool hasz = MOBDB_FLAGS_GET_Z(ts->flags);
  size_t size = sizeof("{'type':'MovingPoint','sequences':[],");
  size += sizeof("{'coordinates':[],'datetimes':[],'lower_inc':false,'upper_inc':false},") * ts->count;
  size += coordinates_mfjson_size(ts->totalcount, hasz, precision);
  size += datetimes_mfjson_size(ts->totalcount);
  /* We reserve space for the largest interpolation string, i.e., "Stepwise" */
  size += sizeof(",interpolations':['Stepwise']}");
  if (srs) size += srs_mfjson_size(srs);
  if (bbox) size += bbox_mfjson_size(hasz, precision);
  return size;
}

/**
 * Writes into the buffer the temporal sequence set point represented in MF-JSON format
 */
static size_t
tpointseqset_as_mfjson_buf(const TSequenceSet *ts, int precision, const STBOX *bbox, char *srs,
  char *output)
{
  char *ptr = output;
  ptr += sprintf(ptr, "{\"type\":\"MovingPoint\",");
  if (srs) ptr += srs_mfjson_buf(ptr, srs);
  if (bbox) ptr += bbox_mfjson_buf(ptr, bbox, MOBDB_FLAGS_GET_Z(ts->flags),
    precision);
  ptr += sprintf(ptr, "\"sequences\":[");
  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    if (i) ptr += sprintf(ptr, ",");
    ptr += sprintf(ptr, "{\"coordinates\":[");
    for (int j = 0; j < seq->count; j++)
    {
      if (j) ptr += sprintf(ptr, ",");
      ptr += coordinates_mfjson_buf(ptr, tsequence_inst_n(seq, j), precision);
    }
    ptr += sprintf(ptr, "],\"datetimes\":[");
    for (int j = 0; j < seq->count; j++)
    {
      if (j) ptr += sprintf(ptr, ",");
      ptr += datetimes_mfjson_buf(ptr, tsequence_inst_n(seq, j));
    }
    ptr += sprintf(ptr, "],\"lower_inc\":%s,\"upper_inc\":%s}",
      seq->period.lower_inc ? "true" : "false", seq->period.upper_inc ? "true" : "false");
  }
  ptr += sprintf(ptr, "],\"interpolations\":[\"%s\"]}",
    MOBDB_FLAGS_GET_LINEAR(ts->flags) ? "Linear" : "Stepwise");
  return (ptr - output);
}

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Return the temporal sequence set point represented in MF-JSON format
 */
char *
tpointseqset_as_mfjson(const TSequenceSet *ts, int precision, const STBOX *bbox,
  char *srs)
{
  size_t size = tpointseqset_as_mfjson_size(ts, precision, bbox, srs);
  char *output = palloc(size);
  tpointseqset_as_mfjson_buf(ts, precision, bbox, srs, output);
  return output;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Return the temporal point represented in MF-JSON format
 */
char *
tpoint_as_mfjson(const Temporal *temp, int precision, int has_bbox, char *srs)
{
  /* Get bounding box if needed */
  STBOX *bbox = NULL, tmp;
  if (has_bbox)
  {
    temporal_bbox(temp, &tmp);
    bbox = &tmp;
  }

  char *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = tpointinst_as_mfjson((TInstant *) temp, precision, bbox, srs);
  else if (temp->subtype == INSTANTSET)
    result = tpointinstset_as_mfjson((TInstantSet *) temp, precision, bbox, srs);
  else if (temp->subtype == SEQUENCE)
    result = tpointseq_as_mfjson((TSequence *) temp, precision, bbox, srs);
  else /* temp->subtype == SEQUENCESET */
    result = tpointseqset_as_mfjson((TSequenceSet *) temp, precision, bbox, srs);
  return result;
}

/*****************************************************************************
 * Output in WKB or EWKB format
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
 * Variants available for WKB and WKT output types
 */
#define WKT_ISO 0x01
// #define WKT_SFSQL 0x02
#define WKT_EXTENDED 0x04

/**
 * Look-up table for hex writer
 */
static char *hexchr = "0123456789ABCDEF";

/**
 * Writes into the buffer the Endian represented in Well-Known Binary (WKB) format
 */
static uint8_t *
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
 * Return true if the bytes must be swaped dependng of the variant
 */
static inline bool
wkb_swap_bytes(uint8_t variant)
{
  /* If requested variant matches machine arch, we don't have to swap! */
#if POSTGIS_VERSION_NUMBER < 30000
  if (((variant & WKB_NDR) && (getMachineEndian() == NDR)) ||
     ((! (variant & WKB_NDR)) && (getMachineEndian() == XDR)))
#else
  if (((variant & WKB_NDR) && !IS_BIG_ENDIAN) ||
      ((!(variant & WKB_NDR)) && IS_BIG_ENDIAN))
#endif
    return false;
  return true;
}

/**
 * Writes into the buffer the Integer32 represented in Well-Known Binary (WKB) format
 */
static uint8_t *
integer_to_wkb_buf(const int ival, uint8_t *buf, uint8_t variant)
{
  char *iptr = (char *)(&ival);

  if (sizeof(int) != WKB_INT_SIZE)
    elog(ERROR, "Machine int size is not %d bytes!", WKB_INT_SIZE);

  if (variant & WKB_HEX)
  {
    int swap = wkb_swap_bytes(variant);
    /* Machine/request arch mismatch, so flip byte order */
    for (int i = 0; i < WKB_INT_SIZE; i++)
    {
      int j = (swap ? WKB_INT_SIZE - 1 - i : i);
      uint8_t b = (uint8_t) iptr[j];
      /* Top four bits to 0-F */
      buf[2*i] = (uint8_t) hexchr[b >> 4];
      /* Bottom four bits to 0-F */
      buf[2*i + 1] = (uint8_t) hexchr[b & 0x0F];
    }
    return buf + (2 * WKB_INT_SIZE);
  }
  else
  {
    /* Machine/request arch mismatch, so flip byte order */
    if (wkb_swap_bytes(variant))
    {
      for (int i = 0; i < WKB_INT_SIZE; i++)
        buf[i] = (uint8_t) iptr[WKB_INT_SIZE - 1 - i];
    }
    /* If machine arch and requested arch match, don't flip byte order */
    else
      memcpy(buf, iptr, WKB_INT_SIZE);
    return buf + WKB_INT_SIZE;
  }
}

/**
 * Writes into the buffer the float64 represented in Well-Known Binary (WKB) format
 */
static uint8_t*
double_to_wkb_buf(double d, uint8_t *buf, uint8_t variant)
{
  char *dptr = (char *)(&d);

  if (sizeof(double) != WKB_DOUBLE_SIZE)
    elog(ERROR, "Machine double size is not %d bytes!", WKB_DOUBLE_SIZE);

  if (variant & WKB_HEX)
  {
    int swap =  wkb_swap_bytes(variant);
    /* Machine/request arch mismatch, so flip byte order */
    for (int i = 0; i < WKB_DOUBLE_SIZE; i++)
    {
      int j = (swap ? WKB_DOUBLE_SIZE - 1 - i : i);
      uint8_t b = (uint8_t) dptr[j];
      /* Top four bits to 0-F */
      buf[2*i] = (uint8_t) hexchr[b >> 4];
      /* Bottom four bits to 0-F */
      buf[2*i + 1] = (uint8_t) hexchr[b & 0x0F];
    }
    return buf + (2 * WKB_DOUBLE_SIZE);
  }
  else
  {
    /* Machine/request arch mismatch, so flip byte order */
    if (wkb_swap_bytes(variant))
    {
      for (int i = 0; i < WKB_DOUBLE_SIZE; i++)
        buf[i] = (uint8_t) dptr[WKB_DOUBLE_SIZE - 1 - i];
    }
    /* If machine arch and requested arch match, don't flip byte order */
    else
      memcpy(buf, dptr, WKB_DOUBLE_SIZE);
    return buf + WKB_DOUBLE_SIZE;
  }
}

/**
 * Writes into the buffer the TimestampTz (aka int64) represented in
 * Well-Known Binary (WKB) format
 */
static uint8_t *
timestamp_to_wkb_buf(TimestampTz t, uint8_t *buf, uint8_t variant)
{
  char *tptr = (char *)(&t);

  if (sizeof(TimestampTz) != WKB_TIMESTAMP_SIZE)
    elog(ERROR, "Machine timestamp size is not %d bytes!", WKB_TIMESTAMP_SIZE);

  if (variant & WKB_HEX)
  {
    int swap =  wkb_swap_bytes(variant);
    /* Machine/request arch mismatch, so flip byte order */
    for (int i = 0; i < WKB_TIMESTAMP_SIZE; i++)
    {
      int j = (swap ? WKB_TIMESTAMP_SIZE - 1 - i : i);
      uint8_t b = (uint8_t) tptr[j];
      /* Top four bits to 0-F */
      buf[2*i] = (uint8_t) hexchr[b >> 4];
      /* Bottom four bits to 0-F */
      buf[2*i + 1] = (uint8_t) hexchr[b & 0x0F];
    }
    return buf + (2 * WKB_TIMESTAMP_SIZE);
  }
  else
  {
    /* Machine/request arch mismatch, so flip byte order */
    if (wkb_swap_bytes(variant))
    {
      for (int i = 0; i < WKB_TIMESTAMP_SIZE; i++)
        buf[i] = (uint8_t) tptr[WKB_TIMESTAMP_SIZE - 1 - i];
    }
    /* If machine arch and requested arch match, don't flip byte order */
    else
      memcpy(buf, tptr, WKB_TIMESTAMP_SIZE);
    return buf + WKB_TIMESTAMP_SIZE;
  }
}

/**
 * Return true if the temporal point needs to output the SRID
 */
static bool
tpoint_wkb_needs_srid(const Temporal *temp, uint8_t variant)
{
  /* Add an SRID if the WKB form is extended and if the geometry has one */
  if ((variant & WKB_EXTENDED) && tpoint_srid(temp) != SRID_UNKNOWN)
    return true;

  /* Everything else doesn't get an SRID */
  return false;
}

/**
 * Return the maximum size in bytes of an array of temporal instant points
 * represented in Well-Known Binary (WKB) format
 */
static size_t
tpointinstarr_to_wkb_size(int npoints, bool hasz)
{
  int dims = hasz ? 3 : 2;
  /* size of the TInstant array */
  size_t size = dims * npoints * WKB_DOUBLE_SIZE + npoints * WKB_TIMESTAMP_SIZE;
  return size;
}

/**
 * Return the maximum size in bytes of the temporal instant point
 * represented in Well-Known Binary (WKB) format
 */
static size_t
tpointinst_to_wkb_size(const TInstant *inst, uint8_t variant)
{
  /* Endian flag + temporal flag */
  size_t size = WKB_BYTE_SIZE * 2;
  /* Extended WKB needs space for optional SRID integer */
  if (tpoint_wkb_needs_srid((Temporal *) inst, variant))
    size += WKB_INT_SIZE;
  /* TInstant */
  size += tpointinstarr_to_wkb_size(1, MOBDB_FLAGS_GET_Z(inst->flags));
  return size;
}

/**
 * Return the maximum size in bytes of the temporal instant set point
 * represented in Well-Known Binary (WKB) format
 */
static size_t
tpointinstset_to_wkb_size(const TInstantSet *ti, uint8_t variant)
{
  /* Endian flag + temporal type flag */
  size_t size = WKB_BYTE_SIZE * 2;
  /* Extended WKB needs space for optional SRID integer */
  if (tpoint_wkb_needs_srid((Temporal *) ti, variant))
    size += WKB_INT_SIZE;
  /* Include the number of instants */
  size += WKB_INT_SIZE;
  /* Include the TInstant array */
  size += tpointinstarr_to_wkb_size(ti->count, MOBDB_FLAGS_GET_Z(ti->flags));
  return size;
}

/**
 * Return the maximum size in bytes of the temporal sequence point
 * represented in Well-Known Binary (WKB) format
 */
static size_t
tpointseq_to_wkb_size(const TSequence *seq, uint8_t variant)
{
  /* Endian flag + temporal type flag */
  size_t size = WKB_BYTE_SIZE * 2;
  /* Extended WKB needs space for optional SRID integer */
  if (tpoint_wkb_needs_srid((Temporal *) seq, variant))
    size += WKB_INT_SIZE;
  /* Include the number of instants and the period bounds flag */
  size += WKB_INT_SIZE + WKB_BYTE_SIZE;
  /* Include the TInstant array */
  size += tpointinstarr_to_wkb_size(seq->count, MOBDB_FLAGS_GET_Z(seq->flags));
  return size;
}

/**
 * Return the maximum size in bytes of the temporal sequence set point
 * represented in Well-Known Binary (WKB) format
 */
static size_t
tpointseqset_to_wkb_size(const TSequenceSet *ts, uint8_t variant)
{
  /* Endian flag + temporal type flag */
  size_t size = WKB_BYTE_SIZE * 2;
  /* Extended WKB needs space for optional SRID integer */
  if (tpoint_wkb_needs_srid((Temporal *) ts, variant))
    size += WKB_INT_SIZE;
  /* Include the number of sequences */
  size += WKB_INT_SIZE;
  /* For each sequence include the number of instants and the period bounds flag */
  size += ts->count * (WKB_INT_SIZE + WKB_BYTE_SIZE);
  /* Include all the TInstant of all the sequences */
  size += tpointinstarr_to_wkb_size(ts->totalcount, MOBDB_FLAGS_GET_Z(ts->flags));
  return size;
}

/**
 * Return the maximum size in bytes of the temporal point
 * represented in Well-Known Binary (WKB) format (dispatch function)
 */
static size_t
tpoint_to_wkb_size(const Temporal *temp, uint8_t variant)
{
  size_t size = 0;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    size = tpointinst_to_wkb_size((TInstant *) temp, variant);
  else if (temp->subtype == INSTANTSET)
    size = tpointinstset_to_wkb_size((TInstantSet *) temp, variant);
  else if (temp->subtype == SEQUENCE)
    size = tpointseq_to_wkb_size((TSequence *) temp, variant);
  else /* temp->subtype == SEQUENCESET */
    size = tpointseqset_to_wkb_size((TSequenceSet *) temp, variant);
  return size;
}

/**
 * Writes into the buffer the flag containing the temporal type and
 * other characteristics represented in Well-Known Binary (WKB) format.
 * In binary format it is a byte as follows
 * LSGZxTTT
 * L = Linear, S = SRID, G = Geodetic, Z = has Z, x = unused bit
 * TTT = Temporal subtype with values 1 to 4
 */
static uint8_t *
tpoint_wkb_type(const Temporal *temp, uint8_t *buf, uint8_t variant)
{
  uint8_t wkb_flags = 0;
  if (MOBDB_FLAGS_GET_Z(temp->flags))
    wkb_flags |= MOBDB_WKB_ZFLAG;
  if (MOBDB_FLAGS_GET_GEODETIC(temp->flags))
    wkb_flags |= MOBDB_WKB_GEODETICFLAG;
  if (tpoint_wkb_needs_srid(temp, variant))
    wkb_flags |= MOBDB_WKB_SRIDFLAG;
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
 * Writes into the buffer the coordinates of the temporal instant point
 * represented in Well-Known Binary (WKB) format as follows
 * - 2 or 3 doubles for the coordinates depending on whether there is Z
 * - 1 timestamp
 */
static uint8_t *
coords_ts_to_wkb_buf(const TInstant *inst, uint8_t *buf, uint8_t variant)
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
  buf = timestamp_to_wkb_buf(inst->t, buf, variant);
  return buf;
}

/**
 * Writes into the buffer the temporal instant point represented in
 * Well-Known Binary (WKB) format as follows
 * - Endian
 * - Linear, SRID, Geodetic, Z, Temporal Subtype
 * - SRID (if requested)
 * - Output of a single instant by function coords_ts_to_wkb_buf
 */
static uint8_t *
tpointinst_to_wkb_buf(const TInstant *inst, uint8_t *buf, uint8_t variant)
{
  /* Set the endian flag */
  buf = endian_to_wkb_buf(buf, variant);
  /* Set the temporal flags */
  buf = tpoint_wkb_type((Temporal *) inst, buf, variant);
  /* Set the optional SRID for extended variant */
  if (tpoint_wkb_needs_srid((Temporal *) inst, variant))
    buf = integer_to_wkb_buf(tpointinst_srid(inst), buf, variant);
  return coords_ts_to_wkb_buf(inst, buf, variant);
}

/**
 * Writes into the buffer the temporal instant set point represented in
 * Well-Known Binary (WKB) format as follows
 * - Endian
 * - Linear, SRID, Geodetic, Z, Temporal Subtype
 * - SRID (if requested)
 * - Number of instants
 * - Output of the instants by function coords_ts_to_wkb_buf
 */
static uint8_t *
tpointinstset_to_wkb_buf(const TInstantSet *ti, uint8_t *buf, uint8_t variant)
{
  /* Set the endian flag */
  buf = endian_to_wkb_buf(buf, variant);
  /* Set the temporal flags */
  buf = tpoint_wkb_type((Temporal *) ti, buf, variant);
  /* Set the optional SRID for extended variant */
  if (tpoint_wkb_needs_srid((Temporal *) ti, variant))
    buf = integer_to_wkb_buf(tpointinstset_srid(ti), buf, variant);
  /* Set the count */
  buf = integer_to_wkb_buf(ti->count, buf, variant);
  /* Set the array of instants */
  for (int i = 0; i < ti->count; i++)
  {
    const TInstant *inst = tinstantset_inst_n(ti, i);
    buf = coords_ts_to_wkb_buf(inst, buf, variant);
  }
  return buf;
}

/**
 * Writes into the buffer the flag containing the bounds represented
 * in Well-Known Binary (WKB) format as follows
 * xxxxxxUL
 * x = Unused bits, U = Upper inclusive, L = Lower inclusive
 */
static uint8_t *
tpointseq_wkb_bounds(const TSequence *seq, uint8_t *buf, uint8_t variant)
{
  uint8_t wkb_flags = 0;
  if (seq->period.lower_inc)
    wkb_flags |= MOBDB_WKB_LOWER_INC;
  if (seq->period.upper_inc)
    wkb_flags |= MOBDB_WKB_UPPER_INC;
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
 * Writes into the buffer the temporal sequence point represented in
 * Well-Known Binary (WKB) format as follows
 * - Endian
 * - Linear, SRID, Geodetic, Z, Temporal Subtype
 * - SRID (if requested)
 * - Number of instants
 * - Lower/upper inclusive
 * - For each instant
 *   - Output of the instant by function coords_ts_to_wkb_buf
 */
static uint8_t *
tpointseq_to_wkb_buf(const TSequence *seq, uint8_t *buf, uint8_t variant)
{
  /* Set the endian flag */
  buf = endian_to_wkb_buf(buf, variant);
  /* Set the temporal flags and interpolation */
  buf = tpoint_wkb_type((Temporal *) seq, buf, variant);
  /* Set the optional SRID for extended variant */
  if (tpoint_wkb_needs_srid((Temporal *) seq, variant))
    buf = integer_to_wkb_buf(tpointseq_srid(seq), buf, variant);
  /* Set the count */
  buf = integer_to_wkb_buf(seq->count, buf, variant);
  /* Set the period bounds */
  buf = tpointseq_wkb_bounds(seq, buf, variant);
  /* Set the array of instants */
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = tsequence_inst_n(seq, i);
    buf = coords_ts_to_wkb_buf(inst, buf, variant);
  }
  return buf;
}

/**
 * Writes into the buffer the temporal sequence set point represented in
 * Well-Known Binary (WKB) format as follows
 * - Endian
 * - Linear, SRID, Geodetic, Z, Temporal Subtype
 * - SRID (if requested)
 * - Number of sequences
 * - For each sequence
 *   - Number or instants
 *   - Lower/upper inclusive
 *   - For each instant of the sequence
 *      - Output of the instant by function coords_ts_to_wkb_buf
 */
static uint8_t *
tpointseqset_to_wkb_buf(const TSequenceSet *ts, uint8_t *buf, uint8_t variant)
{
  /* Set the endian flag */
  buf = endian_to_wkb_buf(buf, variant);
  /* Set the temporal and interpolation flags */
  buf = tpoint_wkb_type((Temporal *) ts, buf, variant);
  /* Set the optional SRID for extended variant */
  if (tpoint_wkb_needs_srid((Temporal *) ts, variant))
    buf = integer_to_wkb_buf(tpointseqset_srid(ts), buf, variant);
  /* Set the count */
  buf = integer_to_wkb_buf(ts->count, buf, variant);
  /* Set the sequences */
  for (int i = 0; i < ts->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ts, i);
    /* Set the number of instants */
    buf = integer_to_wkb_buf(seq->count, buf, variant);
    /* Set the period bounds */
    buf = tpointseq_wkb_bounds(seq, buf, variant);
    /* Set the array of instants */
    for (int j = 0; j < seq->count; j++)
    {
      const TInstant *inst = tsequence_inst_n(seq, j);
      buf = coords_ts_to_wkb_buf(inst, buf, variant);
    }
  }
  return buf;
}

/**
 * Writes into the buffer the temporal point represented in
 * Well-Known Binary (WKB) format
 */
static uint8_t *
tpoint_to_wkb_buf(const Temporal *temp, uint8_t *buf, uint8_t variant)
{
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    return tpointinst_to_wkb_buf((TInstant *) temp, buf, variant);
  else if (temp->subtype == INSTANTSET)
    return tpointinstset_to_wkb_buf((TInstantSet *) temp, buf, variant);
  else if (temp->subtype == SEQUENCE)
    return tpointseq_to_wkb_buf((TSequence *) temp, buf, variant);
  else /* temp->subtype == SEQUENCESET */
    return tpointseqset_to_wkb_buf((TSequenceSet *) temp, buf, variant);
}

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Convert the temporal value to a char * in WKB format.
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
tpoint_to_wkb(const Temporal *temp, uint8_t variant, size_t *size_out)
{
  size_t buf_size;
  uint8_t *buf = NULL;
  uint8_t *wkb_out = NULL;

  /* Initialize output size */
  if (size_out) *size_out = 0;

  /* Calculate the required size of the output buffer */
  buf_size = tpoint_to_wkb_size(temp, variant);

  if (buf_size == 0)
  {
    elog(ERROR, "Error calculating output WKB buffer size.");
    return NULL;
  }

  /* Hex string takes twice as much space as binary + a null character */
  if (variant & WKB_HEX)
  {
    buf_size = 2 * buf_size + 1;
  }

  /* If neither or both variants are specified, choose the native order */
  if (! (variant & WKB_NDR || variant & WKB_XDR) ||
    (variant & WKB_NDR && variant & WKB_XDR))
  {
#if POSTGIS_VERSION_NUMBER < 30000
    if (getMachineEndian() != NDR)
#else
    if (IS_BIG_ENDIAN)
#endif
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
  buf = tpoint_to_wkb_buf(temp, buf, variant);

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
 * @brief Output the temporal point in HexEWKB format.
 * @note This will have 'SRID=#;'
 */
char *
tpoint_as_hexewkb(const Temporal *temp, uint8_t variant, size_t *size)
{
  size_t hexwkb_size;
  /* Create WKB hex string */
  char *result = (char *) tpoint_to_wkb(temp,
    variant | (uint8_t) WKB_EXTENDED | (uint8_t) WKB_HEX, &hexwkb_size);

  *size = hexwkb_size;
  return result;
}

/*****************************************************************************/
/*****************************************************************************/
/*                        MobilityDB - PostgreSQL                            */
/*****************************************************************************/
/*****************************************************************************/

#ifndef MEOS

/*****************************************************************************
 * Output in WKT and EWKT format
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tpoint_as_text);
/**
 * Output a temporal point in Well-Known Text (WKT) format
 */
PGDLLEXPORT Datum
Tpoint_as_text(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  char *str = tpoint_as_text(temp);
  text *result = cstring_to_text(str);
  pfree(str);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEXT_P(result);
}

PG_FUNCTION_INFO_V1(Tpoint_as_ewkt);
/**
 * Output a temporal point in Extended Well-Known Text (EWKT) format,
 * that is, in WKT format prefixed with the SRID
 */
PGDLLEXPORT Datum
Tpoint_as_ewkt(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  char *str = tpoint_as_ewkt(temp);
  text *result = cstring_to_text(str);
  pfree(str);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEXT_P(result);
}

/*****************************************************************************/

/**
 * Output a geometry/geography array in Well-Known Text (WKT) format
 */
static Datum
geoarr_as_text_ext(FunctionCallInfo fcinfo, bool extended)
{
  ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
  /* Return NULL on empty array */
  int count = ArrayGetNItems(ARR_NDIM(array), ARR_DIMS(array));
  if (count == 0)
  {
    PG_FREE_IF_COPY(array, 0);
    PG_RETURN_NULL();
  }

  Datum *geoarr = datumarr_extract(array, &count);
  char **strarr = geoarr_as_text(geoarr, count, extended);
  ArrayType *result = strarr_to_textarray(strarr, count);
  pfree_array((void **) strarr, count);
  pfree(geoarr);
  PG_FREE_IF_COPY(array, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

PG_FUNCTION_INFO_V1(Geoarr_as_text);
/**
 * Output a geometry/geography array in Well-Known Text (WKT) format
 */
PGDLLEXPORT Datum
Geoarr_as_text(PG_FUNCTION_ARGS)
{
  return geoarr_as_text_ext(fcinfo, false);
}

PG_FUNCTION_INFO_V1(Geoarr_as_ewkt);
/**
 * Output a geometry/geography array in Extended Well-Known Text (EWKT) format,
 * that is, in WKT format prefixed with the SRID
 */
PGDLLEXPORT Datum
Geoarr_as_ewkt(PG_FUNCTION_ARGS)
{
  return geoarr_as_text_ext(fcinfo, true);
}

/**
 * Output a temporal point array in Well-Known Text (WKT) or
 * Extended Well-Known Text (EWKT) format
 */
static Datum
tpointarr_as_text_ext(FunctionCallInfo fcinfo, bool extended)
{
  ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
  /* Return NULL on empty array */
  int count = ArrayGetNItems(ARR_NDIM(array), ARR_DIMS(array));
  if (count == 0)
  {
    PG_FREE_IF_COPY(array, 0);
    PG_RETURN_NULL();
  }

  Temporal **temparr = temporalarr_extract(array, &count);
  char **strarr = tpointarr_as_text((const Temporal **) temparr, count,
    extended);
  ArrayType *result = strarr_to_textarray(strarr, count);
  pfree_array((void **) strarr, count);
  pfree(temparr);
  PG_FREE_IF_COPY(array, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

PG_FUNCTION_INFO_V1(Tpointarr_as_text);
/**
 * Output a temporal point array in Well-Known Text (WKT) format
 */
PGDLLEXPORT Datum
Tpointarr_as_text(PG_FUNCTION_ARGS)
{
  return tpointarr_as_text_ext(fcinfo, false);
}

PG_FUNCTION_INFO_V1(Tpointarr_as_ewkt);
/**
 * Output a temporal point array in Extended Well-Known Text (EWKT) format,
 * that is, in WKT format prefixed with the SRID
 */
PGDLLEXPORT Datum
Tpointarr_as_ewkt(PG_FUNCTION_ARGS)
{
  return tpointarr_as_text_ext(fcinfo, true);
}

/*****************************************************************************
 * Output in MFJSON format
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tpoint_as_mfjson);
/**
 * Return the temporal point represented in MF-JSON format
 */
PGDLLEXPORT Datum
Tpoint_as_mfjson(PG_FUNCTION_ARGS)
{
  int has_bbox = 0;
  int precision = DBL_DIG;
  int option = 0;
  char *srs = NULL;

  /* Get the temporal point */
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);

  /* Retrieve precision if any (default is max) */
  if (PG_NARGS() > 1 && !PG_ARGISNULL(1))
  {
    precision = PG_GETARG_INT32(1);
    if (precision > DBL_DIG)
      precision = DBL_DIG;
    else if (precision < 0)
      precision = 0;
  }

  /* Retrieve output option
   * 0 = without option (default)
   * 1 = bbox
   * 2 = short crs
   * 4 = long crs
   */
  if (PG_NARGS() > 2 && !PG_ARGISNULL(2))
    option = PG_GETARG_INT32(2);

  /* Even if the option does not request to output the crs, we output the
   * short crs when the SRID is different from SRID_UNKNOWN. Otherwise,
   * it is not possible to reconstruct the temporal point from the output
   * of this function without loosing the SRID */
  int32_t srid = tpoint_srid(temp);
  if (srid != SRID_UNKNOWN && !(option & 2) && !(option & 4))
    option |= 2;
  if (srid != SRID_UNKNOWN)
  {
    if (option & 2)
      srs = getSRSbySRID(fcinfo, srid, true);
    else if (option & 4)
      srs = getSRSbySRID(fcinfo, srid, false);
    if (! srs)
    {
      elog(ERROR, "SRID %i unknown in spatial_ref_sys table", srid);
      PG_RETURN_NULL();
    }
  }
  if (option & 1)
    has_bbox = 1;

  char *mfjson = tpoint_as_mfjson(temp, precision, has_bbox, srs);
  text *result = cstring_to_text(mfjson);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEXT_P(result);
}

/*****************************************************************************
 * Output in WKB or EWKB format
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
 * Ensures that the spatiotemporal boxes have the same type of coordinates,
 * either planar or geodetic
 */
static void
ensure_valid_endian_flag(const char *endian)
{
  if (strncasecmp(endian, "ndr", 3) != 0 && strncasecmp(endian, "xdr", 3) != 0)
    elog(ERROR, "Invalid value for endian flag");
  return;
}

/**
 * Output the temporal point in WKB or EWKB format
 */
Datum
tpoint_as_binary_ext(FunctionCallInfo fcinfo, bool extended)
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
    tpoint_to_wkb(temp, variant | (uint8_t) WKB_EXTENDED, &wkb_size) :
    tpoint_to_wkb(temp, variant, &wkb_size);

  /* Prepare the PostgreSQL text return type */
  bytea *result = palloc(wkb_size + VARHDRSZ);
  memcpy(VARDATA(result), wkb, wkb_size);
  SET_VARSIZE(result, wkb_size + VARHDRSZ);

  /* Clean up and return */
  pfree(wkb);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BYTEA_P(result);
}

PG_FUNCTION_INFO_V1(Tpoint_as_binary);
/**
 * Output a temporal point in WKB format.
 * This will have no 'SRID=#;'
 */
PGDLLEXPORT Datum
Tpoint_as_binary(PG_FUNCTION_ARGS)
{
  return tpoint_as_binary_ext(fcinfo, false);
}

PG_FUNCTION_INFO_V1(Tpoint_as_ewkb);
/**
 * Output the temporal point in EWKB format.
 * This will have 'SRID=#;'
 */
PGDLLEXPORT Datum
Tpoint_as_ewkb(PG_FUNCTION_ARGS)
{
  return tpoint_as_binary_ext(fcinfo, true);
}

PG_FUNCTION_INFO_V1(Tpoint_as_hexewkb);
/**
 * Output the temporal point in HexEWKB format.
 * This will have 'SRID=#;'
 */
PGDLLEXPORT Datum
Tpoint_as_hexewkb(PG_FUNCTION_ARGS)
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
  char *hexwkb = tpoint_as_hexewkb(temp, variant, &hexwkb_size);

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

#endif /* #ifndef MEOS */

/*****************************************************************************/
