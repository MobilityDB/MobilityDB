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
 * @brief Output of temporal points in WKT, EWKT, and MF-JSON format.
 */

#include "point/tpoint_out.h"

/* C */
#include <assert.h>
#include <float.h>
/* PostGIS */
#if POSTGIS_VERSION_NUMBER >= 30000
  #include <liblwgeom_internal.h>
#endif
/* MobilityDB */
#include <libmeos.h>
#include "general/tinstant.h"
#include "general/tinstantset.h"
#include "general/tsequence.h"
#include "general/tsequenceset.h"
#include "general/temporal_util.h"
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
 * Write into the buffer the coordinate array represented in MF-JSON format
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
#if POSTGIS_VERSION_NUMBER >= 30000
    lwprint_double(pt->x, precision, x);
    lwprint_double(pt->y, precision, y);
    lwprint_double(pt->z, precision, z);
#else
    lwprint_double(pt->x, precision, x, OUT_DOUBLE_BUFFER_SIZE);
    lwprint_double(pt->y, precision, y, OUT_DOUBLE_BUFFER_SIZE);
    lwprint_double(pt->z, precision, z, OUT_DOUBLE_BUFFER_SIZE);
#endif
    ptr += sprintf(ptr, "[%s,%s,%s]", x, y, z);
  }
  else
  {
    const POINT2D *pt = datum_point2d_p(tinstant_value(inst));
#if POSTGIS_VERSION_NUMBER >= 30000
    lwprint_double(pt->x, precision, x);
    lwprint_double(pt->y, precision, y);
#else
    lwprint_double(pt->x, precision, x, OUT_DOUBLE_BUFFER_SIZE);
    lwprint_double(pt->y, precision, y, OUT_DOUBLE_BUFFER_SIZE);
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
 * Write into the buffer the datetimes array represented in MF-JSON format
 */
static size_t
datetimes_mfjson_buf(char *output, const TInstant *inst)
{
  char *ptr = output;
  char *t = basetype_output(T_TIMESTAMPTZ, TimestampTzGetDatum(inst->t));
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
 * Write into the buffer the SRS represented in MF-JSON format
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
 * Write into the buffer the bouding box represented in MF-JSON format
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
  char *begin = basetype_output(T_TIMESTAMPTZ, TimestampTzGetDatum(bbox->tmin));
  char *end = basetype_output(T_TIMESTAMPTZ, TimestampTzGetDatum(bbox->tmax));
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
 * Write into the buffer the temporal instant point represented in MF-JSON format
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
 * @brief Return the MF-JSON representation of a temporal instant point.
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
tpointinstset_as_mfjson_size(const TInstantSet *is, int precision, const STBOX *bbox,
  char *srs)
{
  bool hasz = MOBDB_FLAGS_GET_Z(is->flags);
  size_t size = coordinates_mfjson_size(is->count, hasz, precision);
  size += datetimes_mfjson_size(is->count);
  size += sizeof("{'type':'MovingPoint',");
  size += sizeof("'coordinates':[],'datetimes':[],'interpolations':['Discrete']}");
  if (srs) size += srs_mfjson_size(srs);
  if (bbox) size += bbox_mfjson_size(hasz, precision);
  return size;
}

/**
 * Write into the buffer the temporal instant set point represented in MF-JSON format
 */
static size_t
tpointinstset_as_mfjson_buf(const TInstantSet *is, int precision, const STBOX *bbox,
  char *srs, char *output)
{
  char *ptr = output;
  ptr += sprintf(ptr, "{\"type\":\"MovingPoint\",");
  if (srs) ptr += srs_mfjson_buf(ptr, srs);
  if (bbox) ptr += bbox_mfjson_buf(ptr, bbox, MOBDB_FLAGS_GET_Z(is->flags),
    precision);
  ptr += sprintf(ptr, "\"coordinates\":[");
  for (int i = 0; i < is->count; i++)
  {
    if (i) ptr += sprintf(ptr, ",");
    ptr += coordinates_mfjson_buf(ptr, tinstantset_inst_n(is, i), precision);
  }
  ptr += sprintf(ptr, "],\"datetimes\":[");
  for (int i = 0; i < is->count; i++)
  {
    if (i) ptr += sprintf(ptr, ",");
    ptr += datetimes_mfjson_buf(ptr, tinstantset_inst_n(is, i));
  }
  ptr += sprintf(ptr, "],\"interpolations\":[\"Discrete\"]}");
  return (ptr - output);
}

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Return the MF-JSON representation of a temporal instant set point.
 */
char *
tpointinstset_as_mfjson(const TInstantSet *is, int precision, const STBOX *bbox,
  char *srs)
{
  size_t size = tpointinstset_as_mfjson_size(is, precision, bbox, srs);
  char *output = palloc(size);
  tpointinstset_as_mfjson_buf(is, precision, bbox, srs, output);
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
 * Write into the buffer the temporal sequence point represented in MF-JSON format
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
 * @brief Return the MF-JSON representation of a temporal sequence point.
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
tpointseqset_as_mfjson_size(const TSequenceSet *ss, int precision,
  const STBOX *bbox, char *srs)
{
  bool hasz = MOBDB_FLAGS_GET_Z(ss->flags);
  size_t size = sizeof("{'type':'MovingPoint','sequences':[],");
  size += sizeof("{'coordinates':[],'datetimes':[],'lower_inc':false,'upper_inc':false},") * ss->count;
  size += coordinates_mfjson_size(ss->totalcount, hasz, precision);
  size += datetimes_mfjson_size(ss->totalcount);
  /* We reserve space for the largest interpolation string, i.e., "Stepwise" */
  size += sizeof(",interpolations':['Stepwise']}");
  if (srs) size += srs_mfjson_size(srs);
  if (bbox) size += bbox_mfjson_size(hasz, precision);
  return size;
}

/**
 * Write into the buffer the temporal sequence set point represented in MF-JSON format
 */
static size_t
tpointseqset_as_mfjson_buf(const TSequenceSet *ss, int precision,
  const STBOX *bbox, char *srs, char *output)
{
  char *ptr = output;
  ptr += sprintf(ptr, "{\"type\":\"MovingPoint\",");
  if (srs) ptr += srs_mfjson_buf(ptr, srs);
  if (bbox) ptr += bbox_mfjson_buf(ptr, bbox, MOBDB_FLAGS_GET_Z(ss->flags),
    precision);
  ptr += sprintf(ptr, "\"sequences\":[");
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ss, i);
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
    MOBDB_FLAGS_GET_LINEAR(ss->flags) ? "Linear" : "Stepwise");
  return (ptr - output);
}

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Return the MF-JSON representation of a temporal sequence set point.
 */
char *
tpointseqset_as_mfjson(const TSequenceSet *ss, int precision, const STBOX *bbox,
  char *srs)
{
  size_t size = tpointseqset_as_mfjson_size(ss, precision, bbox, srs);
  char *output = palloc(size);
  tpointseqset_as_mfjson_buf(ss, precision, bbox, srs, output);
  return output;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Return the MF-JSON representation of a temporal point.
 */
char *
tpoint_as_mfjson(const Temporal *temp, int precision, int has_bbox, char *srs)
{
  /* Get bounding box if needed */
  STBOX *bbox = NULL, tmp;
  if (has_bbox)
  {
    temporal_set_bbox(temp, &tmp);
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
  GSERIALIZED *gs = DatumGetGserializedP(value);
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
 * @brief Return the Well-Known Text (WKT) representation of a temporal point.
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
 * @brief Return the Extended Well-Known Text (EWKT) representation a temporal
 * point.
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
 * @brief Return the Well-Known Text (WKT) or the Extended Well-Known Text (EWKT)
 * representation of a geometry/geography array.
 *
 * @param[in] geoarr Array of geometries/geographies
 * @param[in] count Number of elements in the input array
 * @param[in] extended True when the output is in EWKT
 */
char **
geoarr_as_text(const Datum *geoarr, int count, bool extended)
{
  char **result = palloc(sizeof(char *) * count);
  for (int i = 0; i < count; i++)
    /* The wkt_out and ewkt_out functions do not use the first argument */
    result[i] = extended ?
      ewkt_out(0, geoarr[i]) : wkt_out(0, geoarr[i]);
  return result;
}

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Return the Well-Known Text (WKT) or the Extended Well-Known Text (EWKT)
 * representation of a temporal point array
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

/*****************************************************************************/
/*****************************************************************************/
/*                        MobilityDB - PostgreSQL                            */
/*****************************************************************************/
/*****************************************************************************/

#if ! MEOS

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
  text *result = cstring2text(str);
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
  text *result = cstring2text(str);
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
  text *result = cstring2text(mfjson);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEXT_P(result);
}

#endif /* #if ! MEOS */

/*****************************************************************************/
