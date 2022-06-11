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
 * @file temporal_wkt_out.c
 * @brief Output of temporal values in WKT, EWKT, and MF-JSON format.
 */

// #include "general/temporal_wkt_out.h"

/* C */
#include <assert.h>
#include <float.h>
/* PostGIS */
#include <liblwgeom_internal.h>
/* MobilityDB */
#include <libmeos.h>
#include "general/tinstant.h"
#include "general/tinstantset.h"
#include "general/tsequence.h"
#include "general/tsequenceset.h"
#include "general/temporal_util.h"
#include "point/tpoint_spatialfuncs.h"

#define MOBDB_WKT_BOOL_SIZE sizeof("false")
#define MOBDB_WKT_INT4_SIZE sizeof("+2147483647")
#define MOBDB_WKT_INT8_SIZE sizeof("+9223372036854775807")
#define MOBDB_WKT_TIMESTAMPTZ_SIZE sizeof("\"2019-08-06T18:35:48.021455+02:30\"")

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
 * Write into the buffer a base value represented in MF-JSON format.
 */
static size_t
temporal_basevalue_mfjson_size(Datum value, CachedType temptype,
  int precision)
{
  switch (temptype)
  {
    case T_TBOOL:
      return MOBDB_WKT_BOOL_SIZE;
    case T_TINT:
      return MOBDB_WKT_INT4_SIZE;
    case T_TFLOAT:
      assert(precision <= OUT_MAX_DOUBLE_PRECISION);
      return (OUT_MAX_DIGS_DOUBLE + precision);
    case T_TTEXT:
      return VARSIZE_ANY_EXHDR(DatumGetTextP(value)) + sizeof("''") + 1;
    default: /* Error! */
      elog(ERROR, "Unknown temporal type: %d", temptype);
  }
}

/**
 * Write into the buffer an integer represented in MF-JSON format
 */
static size_t
bool_mfjson_buf(char *output, Datum value)
{
  char *ptr = output;
  const bool c = DatumGetInt32(value);
  ptr += sprintf(ptr, "%s", c ? "true" : "false");
  return (ptr - output);
}

/**
 * Write into the buffer an integer represented in MF-JSON format
 */
static size_t
int32_mfjson_buf(char *output, Datum value)
{
  char *ptr = output;
  const int d = DatumGetInt32(value);
  ptr += sprintf(ptr, "%d", d);
  return (ptr - output);
}

/**
 * Write into the buffer the double array represented in MF-JSON format
 */
static size_t
double_mfjson_buf(char *output, Datum value, int precision)
{
  char dstr[OUT_DOUBLE_BUFFER_SIZE];

  assert (precision <= OUT_MAX_DOUBLE_PRECISION);
  char *ptr = output;
  const double d = DatumGetFloat8(value);
  lwprint_double(d, precision, dstr);
  ptr += sprintf(ptr, "%s", dstr);
  return (ptr - output);
}

/**
 * Write into the buffer a text value represented in MF-JSON format
 */
static size_t
text_mfjson_buf(char *output, Datum value)
{
  char *ptr = output;
  char *str = text2cstring(DatumGetTextP(value));
  ptr += sprintf(ptr, "\"%s\"", str);
  pfree(str);
  return (ptr - output);
}

/**
 * Write into the buffer a base value represented in MF-JSON format.
 */
static size_t
temporal_basevalue_mfjson_buf(char *output, Datum value, CachedType temptype,
  int precision)
{
  switch (temptype)
  {
    case T_TBOOL:
      return bool_mfjson_buf(output, value);
    case T_TINT:
      return int32_mfjson_buf(output, value);
    case T_TFLOAT:
      return double_mfjson_buf(output, value, precision);
    case T_TTEXT:
      return text_mfjson_buf(output, value);
    default: /* Error! */
      elog(ERROR, "Unknown temporal type: %d", temptype);
  }
}

/*****************************************************************************/

/**
 * Return the maximum size in bytes of the datetimes array represented
 * in MF-JSON format.
 *
 * For example `"datetimes":["2019-08-06T18:35:48.021455+02:30","2019-08-06T18:45:18.476983+02:30"]`
 * will return 2 enclosing brackets + 1 comma +
 * for each timestamptz 32 characters + 2 double quotes + 1 comma
 */
static size_t
datetimes_mfjson_size(int count)
{
  return (MOBDB_WKT_TIMESTAMPTZ_SIZE + 1) * count;
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
 * Return the maximum size in bytes of the bouding box represented in
 * MF-JSON format
 */
static size_t
bbox_mfjson_size(int precision)
{
  /* The maximum size of a timestamptz is 35 characters, e.g., "2019-08-06 23:18:16.195062-09:30" */
  size_t size = sizeof("'stBoundedBy':{'period':{'begin':,'end':}},") +
    sizeof("\"2019-08-06T18:35:48.021455+02:30\",") * 2;
  size += sizeof("'bbox':[,],");
  size +=  2 * (OUT_MAX_DIGS_DOUBLE + precision);
  return size;
}

/**
 * Write into the buffer the bouding box represented in MF-JSON format
 */
static size_t
bbox_mfjson_buf(char *output, const TBOX *bbox, int precision)
{
  char *ptr = output;
  ptr += sprintf(ptr, "\"stBoundedBy\":{");
  ptr += sprintf(ptr, "\"bbox\":[%.*f,%.*f],",
    precision, bbox->xmin, precision, bbox->xmax);
  char *begin = basetype_output(T_TIMESTAMPTZ, TimestampTzGetDatum(bbox->tmin));
  char *end = basetype_output(T_TIMESTAMPTZ, TimestampTzGetDatum(bbox->tmax));
  ptr += sprintf(ptr, "\"period\":{\"begin\":\"%s\",\"end\":\"%s\"}},", begin, end);
  pfree(begin); pfree(end);
  return (ptr - output);
}

/**
 * Return the maximum size in bytes of the temporal type represented in
 * MF-JSON format
 */
static size_t
temptype_mfjson_size(CachedType temptype)
{
  size_t size;
  ensure_temporal_type(temptype);
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
    default: /* Error! */
      elog(ERROR, "Unknown temporal type: %d", temptype);
      break;
  }
  return size;
}

/**
 * Write into the buffer the temporal type represented in MF-JSON format
 */
static size_t
temptype_mfjson_buf(char *output, CachedType temptype)
{
  char *ptr = output;
  ensure_temporal_type(temptype);
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
    default: /* Error! */
      elog(ERROR, "Unknown temporal type: %d", temptype);
      break;
  }
  return (ptr - output);
}

/*****************************************************************************/

/**
 * Return the maximum size in bytes of a temporal instant represented
 * in MF-JSON format
 */
static size_t
tinstant_as_mfjson_size(const TInstant *inst, int precision, const TBOX *bbox)
{
  Datum value = tinstant_value(inst);
  size_t size = temporal_basevalue_mfjson_size(value, inst->temptype,
    precision);
  size += datetimes_mfjson_size(1);
  size += temptype_mfjson_size(inst->temptype);
  size += sizeof("'values':,'datetimes':,'interpolations':['Discrete']}");
  if (bbox) size += bbox_mfjson_size(precision);
  return size;
}

/**
 * Write into the buffer the temporal instant represented in MF-JSON format
 */
static size_t
tinstant_as_mfjson_buf(const TInstant *inst, int precision, const TBOX *bbox,
  char *output)
{
  char *ptr = output;
  ptr += temptype_mfjson_buf(ptr, inst->temptype);
  if (bbox) ptr += bbox_mfjson_buf(ptr, bbox, precision);
  ptr += sprintf(ptr, "\"values\":");
  ptr += temporal_basevalue_mfjson_buf(ptr, tinstant_value(inst),
    inst->temptype, precision);
  ptr += sprintf(ptr, ",\"datetimes\":");
  ptr += datetimes_mfjson_buf(ptr, inst);
  ptr += sprintf(ptr, ",\"interpolations\":[\"Discrete\"]}");
  return (ptr - output);
}

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Return the MF-JSON representation of a temporal instant.
 * @sqlfunc asMFJSON()
 */
char *
tinstant_as_mfjson(const TInstant *inst, int precision,
  const TBOX *bbox)
{
  size_t size = tinstant_as_mfjson_size(inst, precision, bbox);
  char *output = palloc(size);
  tinstant_as_mfjson_buf(inst, precision, bbox, output);
  return output;
}

/*****************************************************************************/

/**
 * Return the maximum size in bytes of a temporal instant set represented in
 * MF-JSON format
 */
static size_t
tinstantset_as_mfjson_size(const TInstantSet *is, int precision,
  const TBOX *bbox)
{
  size_t size = 0;
  for (int i = 0; i < is->count; i++)
  {
    Datum value = tinstant_value(tinstantset_inst_n(is, i));
    size += temporal_basevalue_mfjson_size(value, is->temptype, precision) +
      sizeof(",");
  }
  size += datetimes_mfjson_size(is->count);
  size += temptype_mfjson_size(is->temptype);
  size += sizeof("'values':[],'datetimes':[],'interpolations':['Discrete']}");
  if (bbox) size += bbox_mfjson_size(precision);
  return size;
}

/**
 * Write into the buffer the temporal instant set point represented in MF-JSON format
 */
static size_t
tinstantset_as_mfjson_buf(const TInstantSet *is, int precision,
  const TBOX *bbox, char *output)
{
  char *ptr = output;
  ptr += temptype_mfjson_buf(ptr, is->temptype);
  if (bbox) ptr += bbox_mfjson_buf(ptr, bbox, precision);
  ptr += sprintf(ptr, "\"values\":[");
  for (int i = 0; i < is->count; i++)
  {
    if (i) ptr += sprintf(ptr, ",");
    const TInstant *inst = tinstantset_inst_n(is, i);
    ptr += temporal_basevalue_mfjson_buf(ptr, tinstant_value(inst),
      inst->temptype, precision);
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
 * @sqlfunc asMFJSON()
 */
char *
tinstantset_as_mfjson(const TInstantSet *is, int precision, const TBOX *bbox)
{
  size_t size = tinstantset_as_mfjson_size(is, precision, bbox);
  char *output = palloc(size);
  tinstantset_as_mfjson_buf(is, precision, bbox, output);
  return output;
}

/*****************************************************************************/

/**
 * Return the maximum size in bytes of a temporal sequence represented in
 * MF-JSON format
 */
static size_t
tsequence_as_mfjson_size(const TSequence *seq, int precision,
  const TBOX *bbox)
{
  size_t size = 0;
  for (int i = 0; i < seq->count; i++)
  {
    Datum value = tinstant_value(tsequence_inst_n(seq, i));
    size += temporal_basevalue_mfjson_size(value, seq->temptype, precision) +
      sizeof(",");
  }
  size += datetimes_mfjson_size(seq->count);
  size += temptype_mfjson_size(seq->temptype);
  /* We reserve space for the largest strings, i.e., 'false' and "Stepwise" */
  size += sizeof("'values':[],'datetimes':[],'lower_inc':false,'upper_inc':false,interpolations':['Stepwise']}");
  if (bbox) size += bbox_mfjson_size(precision);
  return size;
}

/**
 * Write into the buffer the temporal sequence represented in MF-JSON format
 */
static size_t
tsequence_as_mfjson_buf(const TSequence *seq, int precision, const TBOX *bbox,
  char *output)
{
  char *ptr = output;
  ptr += temptype_mfjson_buf(ptr, seq->temptype);
  if (bbox) ptr += bbox_mfjson_buf(ptr, bbox, precision);
  ptr += sprintf(ptr, "\"values\":[");
  for (int i = 0; i < seq->count; i++)
  {
    if (i) ptr += sprintf(ptr, ",");
    const TInstant *inst = tsequence_inst_n(seq, i);
    ptr += temporal_basevalue_mfjson_buf(ptr, tinstant_value(inst),
      inst->temptype, precision);
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
 * @sqlfunc asMFJSON()
 */
char *
tsequence_as_mfjson(const TSequence *seq, int precision, const TBOX *bbox)
{
  size_t size = tsequence_as_mfjson_size(seq, precision, bbox);
  char *output = palloc(size);
  tsequence_as_mfjson_buf(seq, precision, bbox, output);
  return output;
}

/*****************************************************************************/

/**
 * Return the maximum size in bytes of a temporal sequence set
 * represented in MF-JSON format
 */
static size_t
tsequenceset_as_mfjson_size(const TSequenceSet *ss, int precision,
  const TBOX *bbox)
{
  size_t size = temptype_mfjson_size(ss->temptype);
  size += sizeof("'sequences':[],{'values':[],'datetimes':[],'lower_inc':false,'upper_inc':false},") * ss->count;
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ss, i);
    for (int j = 0; j < seq->count; j++)
    {
      Datum value = tinstant_value(tsequence_inst_n(seq, j));
      size += temporal_basevalue_mfjson_size(value, seq->temptype, precision) +
        sizeof(",");
    }
  }
  size += datetimes_mfjson_size(ss->totalcount);
  /* We reserve space for the largest interpolation string, i.e., "Stepwise" */
  size += sizeof(",interpolations':['Stepwise']}");
  if (bbox) size += bbox_mfjson_size(precision);
  return size;
}

/**
 * Write into the buffer the temporal sequence set represented in MF-JSON format
 */
static size_t
tsequenceset_as_mfjson_buf(const TSequenceSet *ss, int precision,
  const TBOX *bbox, char *output)
{
  char *ptr = output;
  ptr += temptype_mfjson_buf(ptr, ss->temptype);
  if (bbox) ptr += bbox_mfjson_buf(ptr, bbox, precision);
  ptr += sprintf(ptr, "\"sequences\":[");
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = tsequenceset_seq_n(ss, i);
    if (i) ptr += sprintf(ptr, ",");
    ptr += sprintf(ptr, "{\"values\":[");
    for (int j = 0; j < seq->count; j++)
    {
      if (j) ptr += sprintf(ptr, ",");
      const TInstant *inst = tsequence_inst_n(seq, j);
      ptr += temporal_basevalue_mfjson_buf(ptr, tinstant_value(inst),
        inst->temptype, precision);
    }
    ptr += sprintf(ptr, "],\"datetimes\":[");
    for (int j = 0; j < seq->count; j++)
    {
      if (j) ptr += sprintf(ptr, ",");
      ptr += datetimes_mfjson_buf(ptr, tsequence_inst_n(seq, j));
    }
    ptr += sprintf(ptr, "],\"lower_inc\":%s,\"upper_inc\":%s}",
      seq->period.lower_inc ? "true" : "false", seq->period.upper_inc ?
        "true" : "false");
  }
  ptr += sprintf(ptr, "],\"interpolations\":[\"%s\"]}",
    MOBDB_FLAGS_GET_LINEAR(ss->flags) ? "Linear" : "Stepwise");
  return (ptr - output);
}

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Return the MF-JSON representation of a temporal sequence set point.
 * @sqlfunc asMFJSON()
 */
char *
tsequenceset_as_mfjson(const TSequenceSet *ss, int precision,
  const TBOX *bbox)
{
  size_t size = tsequenceset_as_mfjson_size(ss, precision, bbox);
  char *output = palloc(size);
  tsequenceset_as_mfjson_buf(ss, precision, bbox, output);
  return output;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Return the MF-JSON representation of a temporal point.
 * @see tinstant_as_mfjson()
 * @see tinstantset_as_mfjson()
 * @see tsequence_as_mfjson()
 * @see tsequenceset_as_mfjson()
 * @sqlfunc asMFJSON()
 */
char *
temporal_as_mfjson(const Temporal *temp, int precision, int has_bbox)
{
  /* Get bounding box if needed */
  TBOX *bbox = NULL, tmp;
  if (has_bbox)
  {
    temporal_set_bbox(temp, &tmp);
    bbox = &tmp;
  }

  char *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = tinstant_as_mfjson((TInstant *) temp, precision, bbox);
  else if (temp->subtype == INSTANTSET)
    result = tinstantset_as_mfjson((TInstantSet *) temp, precision, bbox);
  else if (temp->subtype == SEQUENCE)
    result = tsequence_as_mfjson((TSequence *) temp, precision, bbox);
  else /* temp->subtype == SEQUENCESET */
    result = tsequenceset_as_mfjson((TSequenceSet *) temp, precision, bbox);
  return result;
}

/*****************************************************************************
 * Output in WKT format
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Return the Well-Known Text (WKT) representation of a temporal point.
 */
char *
temporal_as_text(const Temporal *temp)
{
  char *result;
  ensure_valid_tempsubtype(temp->subtype);
  if (temp->subtype == INSTANT)
    result = tinstant_to_string((TInstant *) temp, &basetype_output);
  else if (temp->subtype == INSTANTSET)
    result = tinstantset_to_string((TInstantSet *) temp, &basetype_output);
  else if (temp->subtype == SEQUENCE)
    result = tsequence_to_string((TSequence *) temp, false, &basetype_output);
  else /* temp->subtype == SEQUENCESET */
    result = tsequenceset_to_string((TSequenceSet *) temp, &basetype_output);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Return the Well-Known Text (WKT) or the Extended Well-Known Text (EWKT)
 * representation of a temporal point array
 */
char **
temporalarr_as_text(const Temporal **temparr, int count)
{
  char **result = palloc(sizeof(text *) * count);
  for (int i = 0; i < count; i++)
    result[i] = temporal_as_text(temparr[i]);
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

PG_FUNCTION_INFO_V1(Temporal_as_text);
/**
 * Output a temporal point in Well-Known Text (WKT) format
 */
PGDLLEXPORT Datum
Temporal_as_text(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  char *str = temporal_as_text(temp);
  text *result = cstring2text(str);
  pfree(str);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEXT_P(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Temporalarr_as_text);
/**
 * Output a temporal array in Well-Known Text (WKT) format
 */
PGDLLEXPORT Datum
Temporalarr_as_text(PG_FUNCTION_ARGS)
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
  char **strarr = temporalarr_as_text((const Temporal **) temparr, count);
  ArrayType *result = strarr_to_textarray(strarr, count);
  pfree_array((void **) strarr, count);
  pfree(temparr);
  PG_FREE_IF_COPY(array, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

/*****************************************************************************
 * Output in MFJSON format
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Temporal_as_mfjson);
/**
 * Return the temporal point represented in MF-JSON format
 */
PGDLLEXPORT Datum
Temporal_as_mfjson(PG_FUNCTION_ARGS)
{
  int has_bbox = 0;
  int precision = DBL_DIG;
  int option = 0;

  /* Get the temporal point */
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);

  /* Retrieve output option
   * 0 = without option (default)
   * 1 = bbox
   */
  if (PG_NARGS() > 1 && !PG_ARGISNULL(1))
    option = PG_GETARG_INT32(1);

  if (option & 1)
    has_bbox = 1;

  /* Retrieve precision if any (default is max) */
  if (PG_NARGS() > 2 && !PG_ARGISNULL(2))
  {
    precision = PG_GETARG_INT32(2);
    if (precision > DBL_DIG)
      precision = DBL_DIG;
    else if (precision < 0)
      precision = 0;
  }

  char *mfjson = temporal_as_mfjson(temp, precision, has_bbox);
  text *result = cstring2text(mfjson);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEXT_P(result);
}

#endif /* #if ! MEOS */

/*****************************************************************************/
