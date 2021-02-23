/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2020, Université libre de Bruxelles and MobilityDB
 * contributors
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
 * Output of temporal points in WKT, EWKT, WKB, EWKB, and MF-JSON format
 */

#include "tpoint_out.h"

#include <assert.h>
#include <float.h>
#include <utils/builtins.h>

#include "temporaltypes.h"
#include "oidcache.h"
#include "temporal_util.h"
#include "tpoint.h"
#include "tpoint_spatialfuncs.h"

/* The following definitions are taken from PostGIS */

#define OUT_SHOW_DIGS_DOUBLE 20
#define OUT_MAX_DOUBLE_PRECISION 15
#define OUT_MAX_DIGS_DOUBLE (OUT_SHOW_DIGS_DOUBLE + 2) /* +2 mean add dot and sign */
#define OUT_DOUBLE_BUFFER_SIZE \
  OUT_MAX_DIGS_DOUBLE + OUT_MAX_DOUBLE_PRECISION + 1

/*****************************************************************************
 * Output in WKT and EWKT format
 *****************************************************************************/

/**
 * Output a geometry in Well-Known Text (WKT) format.
 *
 * The Oid argument is not used but is needed since the second argument of
 * the functions temporal*_to_string is of type char *(*value_out)(Oid, Datum)
 */
static char *
wkt_out(Oid type, Datum value)
{
  GSERIALIZED *gs = (GSERIALIZED *)DatumGetPointer(value);
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
 * The Oid argument is not used but is needed since the second argument of
 * the functions temporal*_to_string is of type char *(*value_out)(Oid, Datum)
 */
static char *
ewkt_out(Oid type, Datum value)
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
 * Output a temporal point in Well-Known Text (WKT) format
 * (dispatch function)
 */
static char *
tpoint_as_text_internal1(const Temporal *temp)
{
  char *result;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    result = tinstant_to_string((TInstant *)temp, &wkt_out);
  else if (temp->temptype == INSTANTSET)
    result = tinstantset_to_string((TInstantSet *)temp, &wkt_out);
  else if (temp->temptype == SEQUENCE)
    result = tsequence_to_string((TSequence *)temp, false, &wkt_out);
  else /* temp->temptype == SEQUENCESET */
    result = tsequenceset_to_string((TSequenceSet *)temp, &wkt_out);
  return result;
}

/**
 * Output a temporal point in Well-Known Text (WKT) format
 */
static text *
tpoint_as_text_internal(const Temporal *temp)
{
  char *str = tpoint_as_text_internal1(temp);
  text *result = cstring_to_text(str);
  pfree(str);
  return result;
}

PG_FUNCTION_INFO_V1(tpoint_as_text);
/**
 * Output a temporal point in Well-Known Text (WKT) format
 */
PGDLLEXPORT Datum
tpoint_as_text(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  text *result = tpoint_as_text_internal(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEXT_P(result);
}

/**
 * Output a temporal point in Extended Well-Known Text (EWKT) format,
 * that is, in WKT format prefixed with the SRID (dispatch function)
 */
static text *
tpoint_as_ewkt_internal(const Temporal *temp)
{
  int srid = tpoint_srid_internal(temp);
  char str1[20];
  if (srid > 0)
    sprintf(str1, "SRID=%d%c", srid,
      MOBDB_FLAGS_GET_LINEAR(temp->flags) ? ';' : ',');
  else
    str1[0] = '\0';
  char *str2 = tpoint_as_text_internal1(temp);
  char *str = (char *) palloc(strlen(str1) + strlen(str2) + 1);
  strcpy(str, str1);
  strcat(str, str2);
  text *result = cstring_to_text(str);
  pfree(str2); pfree(str);
  return result;
}

PG_FUNCTION_INFO_V1(tpoint_as_ewkt);
/**
 * Output a temporal point in Extended Well-Known Text (EWKT) format,
 * that is, in WKT format prefixed with the SRID
 */
PGDLLEXPORT Datum
tpoint_as_ewkt(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  text *result = tpoint_as_ewkt_internal(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEXT_P(result);
}

/*****************************************************************************/

/**
 * Output a geometry/geography array in Well-Known Text (WKT) format
 */
Datum
geoarr_as_text1(FunctionCallInfo fcinfo, bool extended)
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
  text **textarr = palloc(sizeof(text *) * count);
  for (int i = 0; i < count; i++)
  {
    /* The wkt_out and ewkt_out functions do not use the first argument */
    char *str = extended ? ewkt_out(ANYOID, geoarr[i]) :
      wkt_out(ANYOID, geoarr[i]);
    textarr[i] = cstring_to_text(str);
    pfree(str);
  }
  ArrayType *result = textarr_to_array(textarr, count, true);

  pfree(geoarr);
  PG_FREE_IF_COPY(array, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

PG_FUNCTION_INFO_V1(geoarr_as_text);
/**
 * Output a geometry/geography array in Well-Known Text (WKT) format
 */
PGDLLEXPORT Datum
geoarr_as_text(PG_FUNCTION_ARGS)
{
  return geoarr_as_text1(fcinfo, false);
}

PG_FUNCTION_INFO_V1(geoarr_as_ewkt);
/**
 * Output a geometry/geography array in Extended Well-Known Text (EWKT) format,
 * that is, in WKT format prefixed with the SRID
 */
PGDLLEXPORT Datum
geoarr_as_ewkt(PG_FUNCTION_ARGS)
{
  return geoarr_as_text1(fcinfo, true);
}

/**
 * Output a temporal point array in Well-Known Text (WKT) or
 * Extended Well-Known Text (EWKT) format
 */
Datum
tpointarr_as_text1(FunctionCallInfo fcinfo, bool extended)
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
  text **textarr = palloc(sizeof(text *) * count);
  for (int i = 0; i < count; i++)
    textarr[i] = extended ? tpoint_as_ewkt_internal(temparr[i]) :
      tpoint_as_text_internal(temparr[i]);
  ArrayType *result = textarr_to_array(textarr, count, true);

  pfree(temparr);
  PG_FREE_IF_COPY(array, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

PG_FUNCTION_INFO_V1(tpointarr_as_text);
/**
 * Output a temporal point array in Well-Known Text (WKT) format
 */
PGDLLEXPORT Datum
tpointarr_as_text(PG_FUNCTION_ARGS)
{
  return tpointarr_as_text1(fcinfo, false);
}

PG_FUNCTION_INFO_V1(tpointarr_as_ewkt);
/**
 * Output a temporal point array in Extended Well-Known Text (EWKT) format,
 * that is, in WKT format prefixed with the SRID
 */
PGDLLEXPORT Datum
tpointarr_as_ewkt(PG_FUNCTION_ARGS)
{
  return tpointarr_as_text1(fcinfo, true);
}

/*****************************************************************************
 * Output in MFJSON format
 *****************************************************************************/

/**
 * Returns the maximum size in bytes of the coordinate array represented in
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
    const POINT3DZ *pt = datum_get_point3dz_p(tinstant_value(inst));
    lwprint_double(pt->x, precision, x, OUT_DOUBLE_BUFFER_SIZE);
    lwprint_double(pt->y, precision, y, OUT_DOUBLE_BUFFER_SIZE);
    lwprint_double(pt->z, precision, z, OUT_DOUBLE_BUFFER_SIZE);
    ptr += sprintf(ptr, "[%s,%s,%s]", x, y, z);
  }
  else
  {
    const POINT2D *pt = datum_get_point2d_p(tinstant_value(inst));
    lwprint_double(pt->x, precision, x, OUT_DOUBLE_BUFFER_SIZE);
    lwprint_double(pt->y, precision, y, OUT_DOUBLE_BUFFER_SIZE);
    ptr += sprintf(ptr, "[%s,%s]", x, y);
  }
  return (ptr - output);
}

/**
 * Returns the maximum size in bytes of the datetimes array represented
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
 * Returns the maximum size in bytes of the SRS represented in MF-JSON format
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
 * Returns the maximum size in bytes of the bouding box represented in
 * MF-JSON format
 */
static size_t
bbox_mfjson_size(int hasz, int precision)
{
  /* The maximum size of a timestamptz is 35 characters, e.g., "2019-08-06 23:18:16.195062-09:30" */
  size_t size = sizeof("'stBoundedBy':{'period':{'begin':,'end':}},") +
    sizeof("\"2019-08-06T18:35:48.021455+02:30\",") * 2;
  if (!hasz)
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
  if (!hasz)
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
 * Returns the maximum size in bytes of a temporal instant point represented
 * in MF-JSON format
 */
static size_t
tpointinst_as_mfjson_size(const TInstant *inst, int precision,
  const STBOX *bbox, char *srs)
{
  size_t size = coordinates_mfjson_size(1,
    MOBDB_FLAGS_GET_Z(inst->flags), precision);
  size += datetimes_mfjson_size(1);
  size += sizeof("{'type':'MovingPoint',");
  size += sizeof("'coordinates':,'datetimes':,'interpolations':['Discrete']}");
  if (srs) size += srs_mfjson_size(srs);
  if (bbox) size += bbox_mfjson_size(MOBDB_FLAGS_GET_Z(inst->flags), precision);
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
  if (bbox) ptr += bbox_mfjson_buf(ptr, bbox,
    MOBDB_FLAGS_GET_Z(inst->flags), precision);
  ptr += sprintf(ptr, "\"coordinates\":");
  ptr += coordinates_mfjson_buf(ptr, inst, precision);
  ptr += sprintf(ptr, ",\"datetimes\":");
  ptr += datetimes_mfjson_buf(ptr, inst);
  ptr += sprintf(ptr, ",\"interpolations\":[\"Discrete\"]}");
  return (ptr - output);
}

/**
 * Returns the temporal instant point represented in MF-JSON format
 */
static char *
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
 * Returns the maximum size in bytes of a temporal instant set point
 * represented in MF-JSON format
 */
static size_t
tpointinstset_as_mfjson_size(const TInstantSet *ti, int precision, const STBOX *bbox,
  char *srs)
{
  size_t size = coordinates_mfjson_size(ti->count,
    MOBDB_FLAGS_GET_Z(ti->flags), precision);
  size += datetimes_mfjson_size(ti->count);
  size += sizeof("{'type':'MovingPoint',");
  size += sizeof("'coordinates':[],'datetimes':[],'interpolations':['Discrete']}");
  if (srs) size += srs_mfjson_size(srs);
  if (bbox) size += bbox_mfjson_size(MOBDB_FLAGS_GET_Z(ti->flags), precision);
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
  if (bbox) ptr += bbox_mfjson_buf(ptr, bbox, MOBDB_FLAGS_GET_Z(ti->flags), precision);
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
 * Returns the temporal instant set point represented in MF-JSON format
 */
static char *
tpointinstset_as_mfjson(const TInstantSet *ti, int precision, const STBOX *bbox, char *srs)
{
  size_t size = tpointinstset_as_mfjson_size(ti, precision, bbox, srs);
  char *output = palloc(size);
  tpointinstset_as_mfjson_buf(ti, precision, bbox, srs, output);
  return output;
}

/*****************************************************************************/

/**
 * Returns the maximum size in bytes of a temporal sequence point
 * represented in MF-JSON format
 */
static size_t
tpointseq_as_mfjson_size(const TSequence *seq, int precision,
  const STBOX *bbox, char *srs)
{
  size_t size = coordinates_mfjson_size(seq->count,
    MOBDB_FLAGS_GET_Z(seq->flags), precision);
  size += datetimes_mfjson_size(seq->count);
  size += sizeof("{'type':'MovingPoint',");
  /* We reserve space for the largest strings, i.e., 'false' and "Stepwise" */
  size += sizeof("'coordinates':[],'datetimes':[],'lower_inc':false,'upper_inc':false,interpolations':['Stepwise']}");
  if (srs) size += srs_mfjson_size(srs);
  if (bbox) size += bbox_mfjson_size(MOBDB_FLAGS_GET_Z(seq->flags), precision);
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
 * Returns the temporal sequence point represented in MF-JSON format
 */
static char *
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
 * Returns the maximum size in bytes of a temporal sequence set point
 * represented in MF-JSON format
 */
static size_t
tpointseqset_as_mfjson_size(const TSequenceSet *ts, int precision, const STBOX *bbox,
  char *srs)
{
  size_t size = sizeof("{'type':'MovingPoint','sequences':[],");
  size += sizeof("{'coordinates':[],'datetimes':[],'lower_inc':false,'upper_inc':false},") * ts->count;
  size += coordinates_mfjson_size(ts->totalcount, MOBDB_FLAGS_GET_Z(ts->flags), precision);
  size += datetimes_mfjson_size(ts->totalcount);
  /* We reserve space for the largest interpolation string, i.e., "Stepwise" */
  size += sizeof(",interpolations':['Stepwise']}");
  if (srs) size += srs_mfjson_size(srs);
  if (bbox) size += bbox_mfjson_size(MOBDB_FLAGS_GET_Z(ts->flags), precision);
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
  if (bbox) ptr += bbox_mfjson_buf(ptr, bbox, MOBDB_FLAGS_GET_Z(ts->flags), precision);
  ptr += sprintf(ptr, "\"sequences\":[");
  for (int i = 0; i < ts->count; i++)
  {
    TSequence *seq = tsequenceset_seq_n(ts, i);
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
 * Returns the temporal sequence set point represented in MF-JSON format
 */
static char *
tpointseqset_as_mfjson(const TSequenceSet *ts, int precision, const STBOX *bbox,
  char *srs)
{
  size_t size = tpointseqset_as_mfjson_size(ts, precision, bbox, srs);
  char *output = palloc(size);
  tpointseqset_as_mfjson_buf(ts, precision, bbox, srs, output);
  return output;
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(tpoint_as_mfjson);
/**
 * Returns the temporal point represented in MF-JSON format
 */
PGDLLEXPORT Datum
tpoint_as_mfjson(PG_FUNCTION_ARGS)
{
  int has_bbox = 0;
  int precision = DBL_DIG;
  char *srs = NULL;

  /* Get the temporal point */
  Temporal *temp = PG_GETARG_TEMPORAL(0);

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
  {
    int option = PG_GETARG_INT32(2);

    if (option & 2 || option & 4)
    {
      int32_t srid = tpoint_srid_internal(temp);
      if (srid != SRID_UNKNOWN)
      {
        if (option & 2)
          srs = getSRSbySRID(fcinfo, srid, true);
          // srs = getSRSbySRID(srid, true);
        if (option & 4)
          srs = getSRSbySRID(fcinfo, srid, false);
          // srs = getSRSbySRID(srid, false);
        if (!srs)
        {
          elog(ERROR, "SRID %i unknown in spatial_ref_sys table",
              srid);
          PG_RETURN_NULL();
        }
      }
    }
    if (option & 1)
      has_bbox = 1;
  }

  /* Get bounding box if needed */
  STBOX *bbox = NULL, tmp;
  memset(&tmp, 0, sizeof(STBOX));
  if (has_bbox)
  {
    temporal_bbox(&tmp, temp);
    bbox = &tmp;
  }

  char *mfjson;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    mfjson = tpointinst_as_mfjson((TInstant *)temp, precision, bbox, srs);
  else if (temp->temptype == INSTANTSET)
    mfjson = tpointinstset_as_mfjson((TInstantSet *)temp, precision, bbox, srs);
  else if (temp->temptype == SEQUENCE)
    mfjson = tpointseq_as_mfjson((TSequence *)temp, precision, bbox, srs);
  else /* temp->temptype == SEQUENCESET */
    mfjson = tpointseqset_as_mfjson((TSequenceSet *)temp, precision, bbox, srs);
  text *result = cstring_to_text(mfjson);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEXT_P(result);
}

/*****************************************************************************
 * Output in WKB format
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
 * Returns true if the bytes must be swaped dependng of the variant
 */
static inline bool
wkb_swap_bytes(uint8_t variant)
{
  /* If requested variant matches machine arch, we don't have to swap! */
  if (((variant & WKB_NDR) && (getMachineEndian() == NDR)) ||
     ((! (variant & WKB_NDR)) && (getMachineEndian() == XDR)))
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
 * Returns true if the temporal point needs to output the SRID
 */
static bool
tpoint_wkb_needs_srid(const Temporal *temp, uint8_t variant)
{
  /* We can only add an SRID if the geometry has one, and the
     WKB form is extended */
  if ((variant & WKB_EXTENDED) && tpoint_srid_internal(temp) != SRID_UNKNOWN)
    return true;

  /* Everything else doesn't get an SRID */
  return false;
}

/**
 * Returns the maximum size in bytes of an array of temporal instant points
 * represented in Well-Known Binary (WKB) format
 */
static size_t
tpointinstarr_to_wkb_size(int npoints, bool hasz, uint8_t variant)
{
  int dims = 2;
  if (hasz)
    dims = 3;
  /* size of the TInstant array */
  size_t size = dims * npoints * WKB_DOUBLE_SIZE + npoints * WKB_TIMESTAMP_SIZE;
  return size;
}

/**
 * Returns the maximum size in bytes of the temporal instant point
 * represented in Well-Known Binary (WKB) format
 */
static size_t
tpointinst_to_wkb_size(const TInstant *inst, uint8_t variant)
{
  /* Endian flag + temporal flag */
  size_t size = WKB_BYTE_SIZE * 2;
  /* Extended WKB needs space for optional SRID integer */
  if (tpoint_wkb_needs_srid((Temporal *)inst, variant))
    size += WKB_INT_SIZE;
  /* TInstant */
  size += tpointinstarr_to_wkb_size(1, MOBDB_FLAGS_GET_Z(inst->flags),
    variant);
  return size;
}

/**
 * Returns the maximum size in bytes of the temporal instant set point
 * represented in Well-Known Binary (WKB) format
 */
static size_t
tpointinstset_to_wkb_size(const TInstantSet *ti, uint8_t variant)
{
  /* Endian flag + temporal type flag */
  size_t size = WKB_BYTE_SIZE * 2;
  /* Extended WKB needs space for optional SRID integer */
  if (tpoint_wkb_needs_srid((Temporal *)ti, variant))
    size += WKB_INT_SIZE;
  /* Include the number of instants */
  size += WKB_INT_SIZE;
  /* Include the TInstant array */
  size += tpointinstarr_to_wkb_size(ti->count, MOBDB_FLAGS_GET_Z(ti->flags),
    variant);
  return size;
}

/**
 * Returns the maximum size in bytes of the temporal sequence point
 * represented in Well-Known Binary (WKB) format
 */
static size_t
tpointseq_to_wkb_size(const TSequence *seq, uint8_t variant)
{
  /* Endian flag + temporal type flag */
  size_t size = WKB_BYTE_SIZE * 2;
  /* Extended WKB needs space for optional SRID integer */
  if (tpoint_wkb_needs_srid((Temporal *)seq, variant))
    size += WKB_INT_SIZE;
  /* Include the number of instants and the period bounds flag */
  size += WKB_INT_SIZE + WKB_BYTE_SIZE;
  /* Include the TInstant array */
  size += tpointinstarr_to_wkb_size(seq->count, MOBDB_FLAGS_GET_Z(seq->flags),
    variant);
  return size;
}

/**
 * Returns the maximum size in bytes of the temporal sequence set point
 * represented in Well-Known Binary (WKB) format
 */
static size_t
tpointseqset_to_wkb_size(const TSequenceSet *ts, uint8_t variant)
{
  /* Endian flag + temporal type flag */
  size_t size = WKB_BYTE_SIZE * 2;
  /* Extended WKB needs space for optional SRID integer */
  if (tpoint_wkb_needs_srid((Temporal *)ts, variant))
    size += WKB_INT_SIZE;
  /* Include the number of sequences */
  size += WKB_INT_SIZE;
  /* For each sequence include the number of instants and the period bounds flag */
  size += ts->count * (WKB_INT_SIZE + WKB_BYTE_SIZE);
  /* Include all the TInstant of all the sequences */
  size += tpointinstarr_to_wkb_size(ts->totalcount, MOBDB_FLAGS_GET_Z(ts->flags),
    variant);
  return size;
}

/**
 * Returns the maximum size in bytes of the temporal point
 * represented in Well-Known Binary (WKB) format (dispatch function)
 */
static size_t
tpoint_to_wkb_size(const Temporal *temp, uint8_t variant)
{
  size_t size = 0;
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    size = tpointinst_to_wkb_size((TInstant *)temp, variant);
  else if (temp->temptype == INSTANTSET)
    size = tpointinstset_to_wkb_size((TInstantSet *)temp, variant);
  else if (temp->temptype == SEQUENCE)
    size = tpointseq_to_wkb_size((TSequence *)temp, variant);
  else /* temp->temptype == SEQUENCESET */
    size = tpointseqset_to_wkb_size((TSequenceSet *)temp, variant);
  return size;
}

/**
 * Writes into the buffer the flag containing the temporal type and 
 * the variant represented in Well-Known Binary (WKB) format
 */
static uint8_t *
tpoint_wkb_type(const Temporal *temp, uint8_t *buf, uint8_t variant)
{
  uint8_t wkb_flags = 0;
  if (variant & WKB_EXTENDED)
  {
    if (MOBDB_FLAGS_GET_Z(temp->flags))
      wkb_flags |= WKB_ZFLAG;
    if (tpoint_wkb_needs_srid(temp, variant))
      wkb_flags |= WKB_SRIDFLAG;
    if (MOBDB_FLAGS_GET_LINEAR(temp->flags))
      wkb_flags |= WKB_LINEAR_INTERP;
  }
  if (variant & WKB_HEX)
  {
    buf[0] = (uint8_t) hexchr[wkb_flags >> 4];
    buf[1] = (uint8_t) hexchr[temp->temptype];
    return buf + 2;
  }
  else
  {
    buf[0] = (uint8_t) temp->temptype + wkb_flags;
    return buf + 1;
  }
}

/**
 * Writes into the buffer the coordinates of the temporal instant point
 * represented in Well-Known Binary (WKB) format
 */
static uint8_t *
coordinates_to_wkb_buf(const TInstant *inst, uint8_t *buf, uint8_t variant)
{
  if (MOBDB_FLAGS_GET_Z(inst->flags))
  {
    const POINT3DZ *point = datum_get_point3dz_p(tinstant_value(inst));
    buf = double_to_wkb_buf(point->x, buf, variant);
    buf = double_to_wkb_buf(point->y, buf, variant);
    buf = double_to_wkb_buf(point->z, buf, variant);
  }
  else
  {
    const POINT2D *point = datum_get_point2d_p(tinstant_value(inst));
    buf = double_to_wkb_buf(point->x, buf, variant);
    buf = double_to_wkb_buf(point->y, buf, variant);
  }
  buf = timestamp_to_wkb_buf(inst->t, buf, variant);
  return buf;
}

/**
 * Writes into the buffer the temporal instant point represented in
 * Well-Known Binary (WKB) format
 */
static uint8_t *
tpointinst_to_wkb_buf(const TInstant *inst, uint8_t *buf, uint8_t variant)
{
  /* Set the endian flag */
  buf = endian_to_wkb_buf(buf, variant);
  /* Set the temporal flags */
  buf = tpoint_wkb_type((Temporal *)inst, buf, variant);
  /* Set the optional SRID for extended variant */
  if (tpoint_wkb_needs_srid((Temporal *)inst, variant))
    buf = integer_to_wkb_buf(tpointinst_srid(inst), buf, variant);
  return coordinates_to_wkb_buf(inst, buf, variant);
}

/**
 * Writes into the buffer the temporal instant set point represented in
 * Well-Known Binary (WKB) format
 */
static uint8_t *
tpointinstset_to_wkb_buf(const TInstantSet *ti, uint8_t *buf, uint8_t variant)
{
  /* Set the endian flag */
  buf = endian_to_wkb_buf(buf, variant);
  /* Set the temporal flags */
  buf = tpoint_wkb_type((Temporal *)ti, buf, variant);
  /* Set the optional SRID for extended variant */
  if (tpoint_wkb_needs_srid((Temporal *)ti, variant))
    buf = integer_to_wkb_buf(tpointinstset_srid(ti), buf, variant);
  /* Set the count */
  buf = integer_to_wkb_buf(ti->count, buf, variant);
  /* Set the array of instants */
  for (int i = 0; i < ti->count; i++)
  {
    TInstant *inst = tinstantset_inst_n(ti, i);
    buf = coordinates_to_wkb_buf(inst, buf, variant);
  }
  return buf;
}

/**
 * Writes into the buffer the flag containing the bounds represented
 * in Well-Known Binary (WKB) format
 */
static uint8_t *
tpointseq_wkb_bounds(const TSequence *seq, uint8_t *buf, uint8_t variant)
{
  uint8_t wkb_flags = 0;
  if (seq->period.lower_inc)
    wkb_flags |= WKB_LOWER_INC;
  if (seq->period.upper_inc)
    wkb_flags |= WKB_UPPER_INC;
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
 * Well-Known Binary (WKB) format
 */
static uint8_t *
tpointseq_to_wkb_buf(const TSequence *seq, uint8_t *buf, uint8_t variant)
{
  /* Set the endian flag */
  buf = endian_to_wkb_buf(buf, variant);
  /* Set the temporal flags and interpolation */
  buf = tpoint_wkb_type((Temporal *)seq, buf, variant);
  /* Set the optional SRID for extended variant */
  if (tpoint_wkb_needs_srid((Temporal *)seq, variant))
    buf = integer_to_wkb_buf(tpointseq_srid(seq), buf, variant);
  /* Set the count */
  buf = integer_to_wkb_buf(seq->count, buf, variant);
  /* Set the period bounds */
  buf = tpointseq_wkb_bounds(seq, buf, variant);
  /* Set the array of instants */
  for (int i = 0; i < seq->count; i++)
  {
    TInstant *inst = tsequence_inst_n(seq, i);
    buf = coordinates_to_wkb_buf(inst, buf, variant);
  }
  return buf;
}

/**
 * Writes into the buffer the temporal sequence set point represented in
 * Well-Known Binary (WKB) format
 */
static uint8_t *
tpointseqset_to_wkb_buf(const TSequenceSet *ts, uint8_t *buf, uint8_t variant)
{
  /* Set the endian flag */
  buf = endian_to_wkb_buf(buf, variant);
  /* Set the temporal and interpolation flags */
  buf = tpoint_wkb_type((Temporal *)ts, buf, variant);
  /* Set the optional SRID for extended variant */
  if (tpoint_wkb_needs_srid((Temporal *)ts, variant))
    buf = integer_to_wkb_buf(tpointseqset_srid(ts), buf, variant);
  /* Set the count */
  buf = integer_to_wkb_buf(ts->count, buf, variant);
  /* Set the sequences */
  for (int i = 0; i < ts->count; i++)
  {
    TSequence *seq = tsequenceset_seq_n(ts, i);
    /* Set the number of instants */
    buf = integer_to_wkb_buf(seq->count, buf, variant);
    /* Set the period bounds */
    buf = tpointseq_wkb_bounds(seq, buf, variant);
    /* Set the array of instants */
    for (int j = 0; j < seq->count; j++)
    {
      TInstant *inst = tsequence_inst_n(seq, j);
      buf = coordinates_to_wkb_buf(inst, buf, variant);
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
  ensure_valid_temptype(temp->temptype);
  if (temp->temptype == INSTANT)
    return tpointinst_to_wkb_buf((TInstant *)temp, buf, variant);
  else if (temp->temptype == INSTANTSET)
    return tpointinstset_to_wkb_buf((TInstantSet *)temp, buf, variant);
  else if (temp->temptype == SEQUENCE)
    return tpointseq_to_wkb_buf((TSequence *)temp, buf, variant);
  else /* temp->temptype == SEQUENCESET */
    return tpointseqset_to_wkb_buf((TSequenceSet *)temp, buf, variant);
}

/**
 * Convert the temporal value to a char* in WKB format. Caller is responsible for freeing
 * the returned array.
 *
 * @param[in] temp Temporal value
 * @param[in] variant Unsigned bitmask value. Accepts one of: WKB_ISO, WKB_EXTENDED, WKB_SFSQL.
 * Accepts any of: WKB_NDR, WKB_HEX. For example: Variant = (WKB_ISO | WKB_NDR) would
 * return the little-endian ISO form of WKB. For Example: Variant = (WKB_EXTENDED | WKB_HEX)
 * would return the big-endian extended form of WKB, as hex-encoded ASCII (the "canonical form").
 * @param[out] size_out If supplied, will return the size of the returned memory segment,
 * including the null terminator in the case of ASCII.
*/
static uint8_t *
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
    if (getMachineEndian() == NDR)
      variant = variant | (uint8_t) WKB_NDR;
    else
      variant = variant | (uint8_t) WKB_XDR;
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
  if (size_out) *size_out = buf_size;

  return wkb_out;
}

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
tpoint_as_binary1(FunctionCallInfo fcinfo, bool extended)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  uint8_t *wkb;
  size_t wkb_size;
  uint8_t variant = 0;
   bytea *result;
  /* If user specified endianness, respect it */
  if ((PG_NARGS() > 1) && (!PG_ARGISNULL(1)))
  {
    text *type = PG_GETARG_TEXT_P(1);
    const char *endian = text_to_cstring(type);
    ensure_valid_endian_flag(endian);
    if (strncasecmp(endian, "ndr", 3) == 0)
      variant = variant | (uint8_t) WKB_NDR;
    else /* type = XDR */
      variant = variant | (uint8_t) WKB_XDR;
  }
  wkb_size = VARSIZE_ANY_EXHDR(temp);
  /* Create WKB hex string */
  wkb = extended ?
    tpoint_to_wkb(temp, variant | (uint8_t) WKB_EXTENDED, &wkb_size) :
    tpoint_to_wkb(temp, variant, &wkb_size);

  /* Prepare the PgSQL text return type */
  result = palloc(wkb_size + VARHDRSZ);
  memcpy(VARDATA(result), wkb, wkb_size);
  SET_VARSIZE(result, wkb_size + VARHDRSZ);

  /* Clean up and return */
  pfree(wkb);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BYTEA_P(result);
}

PG_FUNCTION_INFO_V1(tpoint_as_binary);
/**
 * Output a temporal point in WKB format.
 * This will have no 'SRID=#;'
 */
PGDLLEXPORT Datum
tpoint_as_binary(PG_FUNCTION_ARGS)
{
  return tpoint_as_binary1(fcinfo, false);
}

PG_FUNCTION_INFO_V1(tpoint_as_ewkb);
/**
 * Output the temporal point in EWKB format.
 * This will have 'SRID=#;'
 */
PGDLLEXPORT Datum
tpoint_as_ewkb(PG_FUNCTION_ARGS)
{
  return tpoint_as_binary1(fcinfo, true);
}

PG_FUNCTION_INFO_V1(tpoint_as_hexewkb);
/**
 * Output the temporal point in HexEWKB format.
 * This will have 'SRID=#;'
 */
PGDLLEXPORT Datum
tpoint_as_hexewkb(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  char *hexwkb;
  size_t hexwkb_size;
  uint8_t variant = 0;
  text *result;
  size_t text_size;
  /* If user specified endianness, respect it */
  if ((PG_NARGS() > 1) && (!PG_ARGISNULL(1)))
  {
    text *type = PG_GETARG_TEXT_P(1);
    const char *endian = text_to_cstring(type);
    ensure_valid_endian_flag(endian);
    if (strncasecmp(endian, "ndr", 3) == 0)
      variant = variant | (uint8_t) WKB_NDR;
    else
      variant = variant | (uint8_t) WKB_XDR;
  }

  /* Create WKB hex string */
  hexwkb = (char *)tpoint_to_wkb(temp, variant | (uint8_t) WKB_EXTENDED |
    (uint8_t) WKB_HEX, &hexwkb_size);

  /* Prepare the PgSQL text return type */
  text_size = hexwkb_size - 1 + VARHDRSZ;
  result = palloc(text_size);
  memcpy(VARDATA(result), hexwkb, hexwkb_size - 1);
  SET_VARSIZE(result, text_size);

  /* Clean up and return */
  pfree(hexwkb);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEXT_P(result);
}

/*****************************************************************************/
