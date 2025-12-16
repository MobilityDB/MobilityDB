/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
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
 * @brief Output of types in MF-JSON, WKB, EWKB, and HexWKB representation
 */

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include "utils/timestamp.h"
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif
#include "utils/varlena.h"
/* PostGIS */
#include <liblwgeom.h>
#include <liblwgeom_internal.h>
#include <stringbuffer.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
#include <meos_geo.h>
#include "temporal/set.h"
#include "temporal/span.h"
#include "temporal/spanset.h"
#include "temporal/tbox.h"
#include "temporal/temporal.h"
#include "geo/stbox.h"
#if CBUFFER
  #include "cbuffer/cbuffer.h"
#endif
#if NPOINT
  #include "npoint/tnpoint.h"
#endif
#if POSE
  #include "pose/pose.h"
#endif
#if RGEO
  #include "rgeo/trgeo_all.h"
#endif

#include <utils/jsonb.h>
#include <utils/numeric.h>
#include <pgtypes.h>

#define MEOS_WKT_BOOL_SIZE sizeof("false")
#define MEOS_WKT_INT4_SIZE sizeof("+2147483647")
#define MEOS_WKT_INT8_SIZE sizeof("+9223372036854775807")
#define MEOS_WKT_TIMESTAMPTZ_SIZE sizeof("\"2019-08-06T18:35:48.021455+02:30\",")

/* The following definitions are taken from PostGIS */

#define OUT_SHOW_DIGS_DOUBLE 20
#define OUT_MAX_DOUBLE_PRECISION 15
#define OUT_MAX_DIGS_DOUBLE (OUT_SHOW_DIGS_DOUBLE + 2) /* +2 mean add dot and sign */

/* Functions in lwout_wkb.c */
uint8_t* lwgeom_to_wkb_buf(const LWGEOM *geom, uint8_t *buf, uint8_t variant);
size_t lwgeom_to_wkb_size(const LWGEOM *geom, uint8_t variant);

/*****************************************************************************
 * Output of base types
 *****************************************************************************/

/**
 * @brief Return the string representation of a base value
 * @return On error return @p NULL
 */
char *
basetype_out(Datum value, meosType type, int maxdd)
{
  assert(meos_basetype(type)); assert(maxdd >= 0);

  switch (type)
  {
    case T_TIMESTAMPTZ:
      return pg_timestamptz_out(DatumGetTimestampTz(value));
    case T_DATE:
      return pg_date_out(DatumGetTimestampTz(value));
    case T_BOOL:
      return bool_out(DatumGetBool(value));
    case T_INT4:
      return int32_out(DatumGetInt32(value));
    case T_INT8:
      return int64_out(DatumGetInt64(value));
    case T_FLOAT8:
      return float8_out(DatumGetFloat8(value), maxdd);
    case T_TEXT:
      return text_out(DatumGetTextP(value));
#if DEBUG_BUILD
    case T_DOUBLE2:
      return double2_out(DatumGetDouble2P(value), maxdd);
    case T_DOUBLE3:
      return double3_out(DatumGetDouble3P(value), maxdd);
    case T_DOUBLE4:
      return double4_out(DatumGetDouble4P(value), maxdd);
#endif
    case T_GEOMETRY:
    case T_GEOGRAPHY:
      /* Hex-encoded ASCII Well-Known Binary (HexWKB) representation */
      return geo_out(DatumGetGserializedP(value));
#if CBUFFER
    case T_CBUFFER:
      return cbuffer_out(DatumGetCbufferP(value), maxdd);
#endif
#if NPOINT
    case T_NPOINT:
      return npoint_out(DatumGetNpointP(value), maxdd);
#endif
#if POSE || RGEO
    case T_POSE:
      return pose_out(DatumGetPoseP(value), maxdd);
#endif
    default: /* Error! */
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
        "Unknown output function for type: %s", meostype_name(type));
      return NULL;
  }
}

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
  char *str = text_to_cstring(txt);
  stringbuffer_aprintf(sb, "\"%s\"", str);
  pfree(str);
  return;
}

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
    const POINT3DZ *pt = DATUM_POINT3DZ_P(tinstant_value_p(inst));
    stringbuffer_append_double(sb, pt->x, precision);
    stringbuffer_append_char(sb, ',');
    stringbuffer_append_double(sb, pt->y, precision);
    stringbuffer_append_char(sb, ',');
    stringbuffer_append_double(sb, pt->z, precision);
  }
  else
  {
    const POINT2D *pt = DATUM_POINT2D_P(tinstant_value_p(inst));
    stringbuffer_append_double(sb, pt->x, precision);
    stringbuffer_append_char(sb, ',');
    stringbuffer_append_double(sb, pt->y, precision);
  }
  stringbuffer_append_char(sb, ']');
  return;
}

#if POSE || RGEO
/**
 * @brief Write into the buffer a pose in the MF-JSON representation
 */
static void
pose_as_json_sb(stringbuffer_t *sb, const Pose *pose, int precision)
{
  assert(precision <= OUT_MAX_DOUBLE_PRECISION);
  GSERIALIZED *gs = pose_to_point(pose);
  stringbuffer_append_len(sb, "{\"position\":{", 13);
  if (MEOS_FLAGS_GET_Z(pose->flags))
  {
    const POINT3DZ *pt = GSERIALIZED_POINT3DZ_P(gs);
    stringbuffer_append_len(sb, "\"lat\":", 6);
    stringbuffer_append_double(sb, pt->y, precision);
    stringbuffer_append_len(sb, ",\"lon\":", 7);
    stringbuffer_append_double(sb, pt->x, precision);
    stringbuffer_append_len(sb, ",\"h\":", 5);
    stringbuffer_append_double(sb, pt->z, precision);
    stringbuffer_append_len(sb, "},\"quaternion\":{\"x\":", 20);
    stringbuffer_append_double(sb, pose->data[4], precision);
    stringbuffer_append_len(sb, ",\"y\":", 5);
    stringbuffer_append_double(sb, pose->data[5], precision);
    stringbuffer_append_len(sb, ",\"z\":", 5);
    stringbuffer_append_double(sb, pose->data[6], precision);
    stringbuffer_append_len(sb, ",\"w\":", 5);
    stringbuffer_append_double(sb, pose->data[3], precision);
    stringbuffer_append_char(sb, '}');
  }
  else
  {
    const POINT2D *pt = GSERIALIZED_POINT2D_P(gs);
    stringbuffer_append_len(sb, "\"lat\":", 6);
    stringbuffer_append_double(sb, pt->y, precision);
    stringbuffer_append_len(sb, ",\"lon\":", 7);
    stringbuffer_append_double(sb, pt->x, precision);
    stringbuffer_append_len(sb, "},\"rotation\":", 13);
    stringbuffer_append_double(sb, pose->data[2], precision);
  }
stringbuffer_append_char(sb, '}');
  pfree(gs);
  return;
}
#endif /* POSE */

/**
 * @brief Write into the buffer a base value in the MF-JSON representation
 */
static bool
temporal_base_as_mfjson_sb(stringbuffer_t *sb, Datum value, meosType temptype,
  int precision)
{
  assert(alphanum_temptype(temptype));
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
    case T_TGEOGRAPHY:
    case T_TGEOMETRY:
    {
      char *str = geo_as_geojson(DatumGetGserializedP(value), 0, precision,
        NULL);
      stringbuffer_aprintf(sb, "%s,", str);
      break;
    }
    default: /* Error! */
      meos_error(ERROR, MEOS_ERR_MFJSON_OUTPUT,
        "Unknown temporal type in MFJSON output: %s",
        meostype_name(temptype));
      return false;
  }
  return true;
}

/*****************************************************************************/

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
srs_as_mfjson_sb(stringbuffer_t *sb, const char *srs)
{
  stringbuffer_append_len(sb, "\"crs\":{\"type\":\"Name\",", 21);
  stringbuffer_aprintf(sb, "\"properties\":{\"name\":\"%s\"}},", srs);
  return;
}

/**
 * @brief Write into the buffer a tstzspan in the MF-JSON representation
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
 * @brief Write into the buffer a temporal box in the MF-JSON representation
 */
static void
tbox_as_mfjson_sb(stringbuffer_t *sb, const TBox *box, int precision)
{
  assert(precision <= OUT_MAX_DOUBLE_PRECISION);
  bool intbox = box->span.basetype == T_INT4;
  stringbuffer_append_len(sb, "\"bbox\":[", 8);
  if (intbox)
    stringbuffer_aprintf(sb, "%d", DatumGetInt32(box->span.lower));
  else
    stringbuffer_append_double(sb, DatumGetFloat8(box->span.lower), precision);
  stringbuffer_append_char(sb, ',');
  if (intbox)
    stringbuffer_aprintf(sb, "%d", DatumGetInt32(box->span.upper) - 1);
  else
    stringbuffer_append_double(sb, DatumGetFloat8(box->span.upper), precision);
  stringbuffer_append_len(sb, "],", 2);
  tstzspan_as_mfjson_sb(sb, &box->period);
  return;
}

/**
 * @brief Write into the buffer a spatiotemporal box in the MF-JSON
 * representation
 */
static void
stbox_as_mfjson_sb(stringbuffer_t *sb, const STBox *box, int precision)
{
  assert(precision <= OUT_MAX_DOUBLE_PRECISION);
  stringbuffer_append_len(sb, "\"bbox\":[[", 9);
  stringbuffer_append_double(sb, box->xmin, precision);
  stringbuffer_append_char(sb, ',');
  stringbuffer_append_double(sb, box->ymin, precision);
  bool hasz = MEOS_FLAGS_GET_Z(box->flags);
  if (hasz)
  {
    stringbuffer_append_char(sb, ',');
    stringbuffer_append_double(sb, box->zmin, precision);
  }
  stringbuffer_append_len(sb, "],[", 3);
  stringbuffer_append_double(sb, box->xmax, precision);
  stringbuffer_append_char(sb, ',');
  stringbuffer_append_double(sb, box->ymax, precision);
  if (hasz)
  {
    stringbuffer_append_char(sb, ',');
    stringbuffer_append_double(sb, box->zmax, precision);
  }
  stringbuffer_append_len(sb, "]],", 3);
  tstzspan_as_mfjson_sb(sb, &box->period);
  return;
}

/**
 * @brief Write into the buffer a bounding box corresponding to the temporal
 * type in the MF-JSON representation
 */
static bool
bbox_as_mfjson_sb(stringbuffer_t *sb, meosType temptype, const bboxunion *box,
  int precision)
{
  assert(temporal_type(temptype));
  switch (temptype)
  {
    case T_TBOOL:
    case T_TTEXT:
      tstzspan_as_mfjson_sb(sb, (Span *) box);
      break;
    case T_TINT:
    case T_TFLOAT:
      tbox_as_mfjson_sb(sb, (TBox *) box, precision);
      break;
    case T_TGEOMPOINT:
    case T_TGEOGPOINT:
    case T_TGEOMETRY:
    case T_TGEOGRAPHY:
    case T_TPOSE:
    case T_TRGEOMETRY:
      stbox_as_mfjson_sb(sb, (STBox *) box, precision);
      break;
    default: /* Error! */
      meos_error(ERROR, MEOS_ERR_MFJSON_OUTPUT,
        "Unknown temporal type in MFJSON output: %s",
        meostype_name(temptype));
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
    case T_TGEOMETRY:
    case T_TGEOGRAPHY:
      stringbuffer_append_len(sb, "{\"type\":\"MovingGeometry\",", 25);
      break;
#if POSE
    case T_TPOSE:
      stringbuffer_append_len(sb, "{\"type\":\"MovingPose\",", 21);
      break;
#endif
#if RGEO
    case T_TRGEOMETRY:
      stringbuffer_append_len(sb, "{\"type\":\"MovingRigidGeometry\",", 30);
      break;
#endif
    default: /* Error! */
      meos_error(ERROR, MEOS_ERR_MFJSON_OUTPUT,
        "Unknown temporal type in MFJSON output: %s",
        meostype_name(temptype));
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
tinstant_as_mfjson_sb(stringbuffer_t *sb, const TInstant *inst,
  const bboxunion *bbox, int precision, const char *srs)
{
  bool success = temptype_as_mfjson_sb(sb, inst->temptype);
  /* Propagate errors up */
  if (! success)
    return false;
  if (srs)
    srs_as_mfjson_sb(sb, srs);
  if (bbox)
  {
    success = bbox_as_mfjson_sb(sb, inst->temptype, bbox, precision);
    /* Propagate errors up */
    if (! success)
      return false;
  }
  if (tpoint_type(inst->temptype))
  {
    stringbuffer_append_len(sb, "\"coordinates\":[", 15);
    coordinates_as_mfjson_sb(sb, inst, precision);
  }
  else if (tgeo_type(inst->temptype))
  {
    const GSERIALIZED *gs = DatumGetGserializedP(tinstant_value_p(inst));
    /* Do not repeat the crs for the composing geometries */
    char *str = geo_as_geojson(gs, 0, precision, NULL);
    stringbuffer_aprintf(sb, "\"values\":[%s", str);
    pfree(str);
  }
#if POSE
  else if (inst->temptype == T_TPOSE)
  {
    /* Do not repeat the crs for the composing geometries */
    stringbuffer_append_len(sb, "\"values\":[", 10);
    pose_as_json_sb(sb, DatumGetPoseP(tinstant_value_p(inst)), precision);
  }
#endif /* POSE */
#if RGEO
  else if (inst->temptype == T_TRGEOMETRY)
  {
    /* Do not repeat the crs for the composing geometries */
    stringbuffer_append_len(sb, "\"geometry\":", 11);
    const GSERIALIZED *gs = trgeoinst_geom_p(inst);
    char *str = geo_as_geojson(gs, 0, precision, NULL);
    stringbuffer_aprintf(sb, "%s,", str);
    stringbuffer_append_len(sb, "\"values\":[", 10);
    pose_as_json_sb(sb, DatumGetPoseP(tinstant_value_p(inst)), precision);
  }
#endif /* RGEO */
  else
  {
    stringbuffer_append_len(sb, "\"values\":[", 10);
    success = temporal_base_as_mfjson_sb(sb, tinstant_value_p(inst),
      inst->temptype, precision);
    /* Propagate errors up */
    if (! success)
      return false;
  }
  stringbuffer_append_len(sb, "],\"datetimes\":[", 15);
  datetimes_as_mfjson_sb(sb, inst->t);
  stringbuffer_append_len(sb, "],\"interpolation\":\"None\"}", 25);
  return true;
}

/**
 * @brief Write into the buffer a temporal sequence in the MF-JSON
 * representation
 */
static bool
tsequence_as_mfjson_sb(stringbuffer_t *sb, const TSequence *seq,
  const bboxunion *box, int precision, const char *srs)
{
  bool success = temptype_as_mfjson_sb(sb, seq->temptype);
  /* Propagate errors up */
  if (! success)
    return false;
  if (srs)
    srs_as_mfjson_sb(sb, srs);
  if (box)
  {
    success = bbox_as_mfjson_sb(sb, seq->temptype, box, precision);
    /* Propagate errors up */
    if (! success)
      return false;
  }
  if (tpoint_type(seq->temptype))
    stringbuffer_append_len(sb, "\"coordinates\":[", 15);
#if RGEO
  else if (seq->temptype == T_TRGEOMETRY)
  {
    stringbuffer_append_len(sb, "\"geometry\":", 11);
    const GSERIALIZED *gs = trgeoseq_geom_p(seq);
    char *str = geo_as_geojson(gs, 0, precision, NULL);
    stringbuffer_aprintf(sb, "%s,\"values\":[", str);
  }
#endif /* RGEO */
  else
    stringbuffer_append_len(sb, "\"values\":[", 10);
  const TInstant *inst;
  for (int i = 0; i < seq->count; i++)
  {
    if (i)
      stringbuffer_append_char(sb, ',');
    inst = TSEQUENCE_INST_N(seq, i);
    if (tpoint_type(inst->temptype))
      coordinates_as_mfjson_sb(sb, inst, precision);
    else if (tgeo_type(inst->temptype))
    {
      const GSERIALIZED *gs = DatumGetGserializedP(tinstant_value_p(inst));
      /* Do not repeat the crs for the composing geometries */
      char *str = geo_as_geojson(gs, 0, precision, NULL);
      stringbuffer_aprintf(sb, "%s", str);
      pfree(str);
    }
#if POSE
    else if (inst->temptype == T_TPOSE)
    {
      /* Do not repeat the crs for the composing geometries */
      pose_as_json_sb(sb, DatumGetPoseP(tinstant_value_p(inst)), precision);
    }
#endif /* POSE */
#if RGEO
    else if (inst->temptype == T_TRGEOMETRY)
    {
      /* Do not repeat the crs of the reference geometry for the instants */
      pose_as_json_sb(sb, DatumGetPoseP(tinstant_value_p(inst)), precision);
    }
#endif /* RGEO */
    else
    {
      success = temporal_base_as_mfjson_sb(sb, tinstant_value_p(inst), 
        inst->temptype, precision);
      /* Propagate errors up */
      if (! success)
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

/*****************************************************************************/

/**
 * @brief Write into the buffer a temporal sequence set in the MF-JSON
 * representation
 */
static bool
tsequenceset_as_mfjson_sb(stringbuffer_t *sb, const TSequenceSet *ss,
  const bboxunion *box, int precision, const char *srs)
{
  bool success = temptype_as_mfjson_sb(sb, ss->temptype);
  /* Propagate errors up */
  if (! success)
    return false;
  if (srs)
    srs_as_mfjson_sb(sb, srs);
  if (box)
  {
    success = bbox_as_mfjson_sb(sb, ss->temptype, box, precision);
    /* Propagate errors up */
    if (! success)
      return false;
  }

#if RGEO
  if (ss->temptype == T_TRGEOMETRY)
  {
    stringbuffer_append_len(sb, "\"geometry\":", 11);
    char *str = geo_as_geojson(trgeoseqset_geom_p(ss), 0, precision, NULL);
    stringbuffer_aprintf(sb, "%s,", str);
  }
#endif /* RGEO */

  stringbuffer_append_len(sb, "\"sequences\":[", 13);
  const TInstant *inst;
  for (int i = 0; i < ss->count; i++)
  {
    const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
    if (i)
      stringbuffer_append_char(sb, ',');
    if (tpoint_type(seq->temptype))
      stringbuffer_append_len(sb, "{\"coordinates\":[", 16);
    else
      stringbuffer_append_len(sb, "{\"values\":[", 11);
    for (int j = 0; j < seq->count; j++)
    {
      if (j)
        stringbuffer_append_char(sb, ',');
      inst = TSEQUENCE_INST_N(seq, j);
      if (tpoint_type(inst->temptype))
        coordinates_as_mfjson_sb(sb, inst, precision);
      else if (tgeo_type(inst->temptype))
      {
        const GSERIALIZED *gs = DatumGetGserializedP(tinstant_value_p(inst));
        /* Do not repeat the crs for the composing geometries */
        char *str = geo_as_geojson(gs, 0, precision, NULL);
        stringbuffer_aprintf(sb, "%s", str);      
        // pfree(str);
      }
#if POSE
      else if (inst->temptype == T_TPOSE)
      {
        /* Do not repeat the crs for the composing geometries */
        pose_as_json_sb(sb, DatumGetPoseP(tinstant_value_p(inst)), precision);
      }
#endif /* POSE */
#if RGEO
      else if (inst->temptype == T_TRGEOMETRY)
      {
        /* Do not repeat the crs of the reference geometry for the instants */
        pose_as_json_sb(sb, DatumGetPoseP(tinstant_value_p(inst)), precision);
      }
#endif /* RGEO */
      else
      {
        success = temporal_base_as_mfjson_sb(sb, tinstant_value_p(inst),
          inst->temptype, precision);
        /* Propagate errors up */
        if (! success)
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
      seq->period.lower_inc ? "true" : "false", 
      seq->period.upper_inc ? "true" : "false");
  }
  stringbuffer_aprintf(sb, "],\"interpolation\":\"%s\"}",
    MEOS_FLAGS_LINEAR_INTERP(ss->flags) ? "Linear" : "Step");
  return true;
}

/*****************************************************************************/

/**
 * @ingroup meos_temporal_inout
 * @brief Return the MF-JSON representation of a temporal value
 * @param[in] temp Temporal value
 * @param[in] with_bbox True when the output value has bounding box
 * @param[in] flags Flags
 * @param[in] precision Number of decimal digits. It is only used when the base
 * type has floating point components, such as tfloat or tgeometry
 * @param[in] srs Spatial reference system
 * @return On error return @p NULL
 * @csqlfn #Temporal_as_mfjson()
 */
char *
temporal_as_mfjson(const Temporal *temp, bool with_bbox, int flags,
  int precision, const char *srs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL);

  /* Get bounding box if needed */
  bboxunion *box = NULL, tmp;
  if (with_bbox)
  {
    temporal_set_bbox(temp, &tmp);
    box = &tmp;
  }

  /* Create the string buffer */
  stringbuffer_t *sb = stringbuffer_create();

  bool res;
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      res = tinstant_as_mfjson_sb(sb, (TInstant *) temp, box, precision, srs);
      break;
    case TSEQUENCE:
      res = tsequence_as_mfjson_sb(sb, (TSequence *) temp, box, precision, 
        srs);
      break;
    default: /* TSEQUENCESET */
      res = tsequenceset_as_mfjson_sb(sb, (TSequenceSet *) temp, box,
        precision, srs);
  }
  /* Convert the string buffer to a C string */
  char *result = ! res ? NULL : stringbuffer_getstringcopy(sb);

  /* Destroy the string buffer */
  stringbuffer_destroy(sb);

  if (flags == 0)
    return result;

  /* Convert to JSON and back to a C string to apply flags using json-c */
  json_tokener *jstok = json_tokener_new();
  struct json_object *jobj = json_tokener_parse_ex(jstok, result, -1);
  pfree(result);
  result = strdup(json_object_to_json_string_ext(jobj, flags));
  json_tokener_free(jstok);
  json_object_put(jobj);
  return result;
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
 * Determine the size of the WKB representation
 *****************************************************************************/

/**
 * @brief Return true if a circular buffer needs to output the SRID in the
 * Well-Known Binary (WKB) representation
 */
static bool
spatial_wkb_needs_srid(int32_t srid, uint8_t variant)
{
  /* Add an SRID if the WKB form is extended and if the srid received is known */
  if ((variant & WKB_EXTENDED) && srid != SRID_UNKNOWN)
    return true;
  /* Everything else doesn't get an SRID */
  return false;
}

/**
 * @brief Return the size of the WKB representation of the geo value
 * @note Since the geo is embedded in a container such as a set or a temporal
 * value, the SRID value (if any) is only output for the container
 */
static size_t
geo_to_wkb_size(const GSERIALIZED *gs, uint8_t variant)
{
  LWGEOM *geo = lwgeom_from_gserialized(gs);
  size_t result = lwgeom_to_wkb_size(geo, variant);
  lwgeom_free(geo);
  return result;
}

#if CBUFFER
/**
 * @brief Return the size in bytes of a circular buffer in the Well-Known
 * Binary (WKB) representation
 */
static size_t
cbuffer_to_wkb_size(const Cbuffer *cb, uint8_t variant, bool component)
{
  size_t size = 0;
  if (! component)
  {
    /* Endian flag + SRID flag + optional SRID */
    size += MEOS_WKB_BYTE_SIZE * 2;
    if (spatial_wkb_needs_srid(cbuffer_srid(cb), variant))
      size += MEOS_WKB_INT4_SIZE;
  }
  /* x and y coordinates + radius */
  size += MEOS_WKB_DOUBLE_SIZE * 3;
  return size;
}
#endif /* CBUFFER */

#if NPOINT
/**
 * @brief Return the size in bytes of a network point in the Well-Known
 * Binary (WKB) representation
 */
static size_t
npoint_to_wkb_size(const Npoint *np, uint8_t variant, bool component)
{
  size_t size = 0;
  if (! component)
    /* Endian flag */
    size += MEOS_WKB_BYTE_SIZE;
  /* rid  + position */
  size += MEOS_WKB_BYTE_SIZE +  MEOS_WKB_INT8_SIZE + MEOS_WKB_DOUBLE_SIZE;
  if (spatial_wkb_needs_srid(npoint_srid(np), variant))
    size += MEOS_WKB_INT4_SIZE;
  return size;
}
#endif /* NPOINT */

#if POSE || RGEO
/**
 * @brief Return the size in bytes of a pose in the Well-Known Binary (WKB)
 * representation
 */
static size_t
pose_to_wkb_size(const Pose *pose, uint8_t variant, bool component)
{
  /* Pose flags (1 byte) */
  size_t size = 1;
  if (! component)
    /* Endian flag */
    size += MEOS_WKB_BYTE_SIZE;
  /* 2D: 3 double values, 3D: 7 double values */
  size +=  MEOS_FLAGS_GET_Z(pose->flags) ?
    MEOS_WKB_DOUBLE_SIZE * 7 : MEOS_WKB_DOUBLE_SIZE * 3;
  if (spatial_wkb_needs_srid(pose_srid(pose), variant))
    size += MEOS_WKB_INT4_SIZE;
  return size;
}
#endif /* POSE */

/**
 * @brief Return the size of the WKB representation of a base value
 * @return On error return SIZE_MAX
 */
static size_t
base_to_wkb_size(Datum value, meosType basetype, uint8_t variant)
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
      return MEOS_WKB_INT8_SIZE + VARSIZE_ANY_EXHDR(DatumGetTextP(value));
    case T_GEOMETRY:
    case T_GEOGRAPHY:
      return geo_to_wkb_size(DatumGetGserializedP(value), variant);
#if CBUFFER
    case T_CBUFFER:
      return cbuffer_to_wkb_size(DatumGetCbufferP(value), variant, true);
#endif /* CBUFFER */
#if NPOINT
    case T_NPOINT:
      return npoint_to_wkb_size(DatumGetNpointP(value), variant, true);
#endif /* NPOINT */
#if POSE || RGEO
    case T_POSE:
      return pose_to_wkb_size(DatumGetPoseP(value), variant, true);
#endif /* POSE || RGEO */
    default: /* Error! */
      meos_error(ERROR, MEOS_ERR_MFJSON_OUTPUT,
        "Unknown temporal base type in WKB output: %s",
        meostype_name(basetype));
      return SIZE_MAX;
  }
}

/**
 * @brief Return the size in bytes of a set in the Well-Known Binary
 * (WKB) representation
 */
static size_t
set_to_wkb_size(const Set *set, uint8_t variant)
{
  /* Endian flag (byte) + settype (int2) + set flags (byte) + count (int4) */
  size_t result = MEOS_WKB_BYTE_SIZE * 2 + MEOS_WKB_INT2_SIZE +
    MEOS_WKB_INT4_SIZE;
  if (spatialset_type(set->settype) &&
      spatial_wkb_needs_srid(spatialset_srid(set), variant))
    result += MEOS_WKB_INT4_SIZE;
  /* Compute the size of the values which may be of variable length*/
  for (int i = 0; i < set->count; i++)
    result += base_to_wkb_size(SET_VAL_N(set, i), set->basetype, variant);
  return result;
}

/*****************************************************************************/

/**
 * @brief Return the size in bytes of a span in the Well-Known Binary (WKB)
 * representation
 */
size_t
span_to_wkb_size(const Span *s, bool component)
{
  size_t size = 0;
  if (! component)
    /* Write the endian flag (byte) */
    size += MEOS_WKB_BYTE_SIZE;
  /* spantype + bounds flag + basetype values */
  size += MEOS_WKB_INT2_SIZE + MEOS_WKB_BYTE_SIZE +
    /* Only the second parameter is used for spans */
    base_to_wkb_size(0, s->basetype, 0) * 2;
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
  size_t sizebase = base_to_wkb_size(0, ss->basetype, 0);
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
    size += span_to_wkb_size(&box->span, true);
  /* If there is a time dimension */
  if (MEOS_FLAGS_GET_T(box->flags))
    size += span_to_wkb_size(&box->period, true);
  return size;
}

/*****************************************************************************/

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
    size += span_to_wkb_size(&box->period, true);
  /* If there is a value dimension */
  if (MEOS_FLAGS_GET_X(box->flags))
  {
    if (spatial_wkb_needs_srid(box->srid, variant))
      size += MEOS_WKB_INT4_SIZE;
    size += MEOS_WKB_DOUBLE_SIZE * 4;
    if (MEOS_FLAGS_GET_Z(box->flags))
      size += MEOS_WKB_DOUBLE_SIZE * 2;
  }
  return size;
}

/*****************************************************************************/

/**
 * @brief Return the maximum size in bytes of an array of temporal instants
 * in the Well-Known Binary (WKB) representation
 */
static size_t
tinstarr_to_wkb_size(TInstant **instants, int count, uint8_t variant)
{
  size_t result = 0;
  meosType basetype = temptype_basetype(instants[0]->temptype);
  for (int i = 0; i < count; i++)
  {
    Datum value = tinstant_value_p(instants[i]);
    result += base_to_wkb_size(value, basetype, variant);
  }
  /* size of the TInstant array */
  result += count * MEOS_WKB_TIMESTAMP_SIZE;
  return result;
}

/**
 * @brief Return the maximum size in bytes of the temporal instant in the
 * Well-Known Binary (WKB) representation
 */
static size_t
tinstant_to_wkb_size(const TInstant *inst, uint8_t variant)
{
  /* Endian flag (byte) + temporal type (int2) + temporal flag (byte) */
  size_t result = MEOS_WKB_BYTE_SIZE * 2 + MEOS_WKB_INT2_SIZE;
  /* Extended WKB needs space for optional SRID integer */
  if (tspatial_type(inst->temptype) &&
      spatial_wkb_needs_srid(tspatial_srid((Temporal *) inst), variant))
    result += MEOS_WKB_INT4_SIZE;
  /* TInstant */
  result += tinstarr_to_wkb_size((TInstant **) &inst, 1, variant);
  return result;
}

/**
 * @brief Return the maximum size in bytes of the temporal sequence in the
 * Well-Known Binary (WKB) representation
 */
static size_t
tsequence_to_wkb_size(const TSequence *seq, uint8_t variant)
{
  /* Endian flag + temporal type + temporal flag */
  size_t result = MEOS_WKB_BYTE_SIZE * 2 + MEOS_WKB_INT2_SIZE;
  /* Extended WKB needs space for optional SRID integer */
  if (tspatial_type(seq->temptype) &&
      spatial_wkb_needs_srid(tspatial_srid((Temporal *) seq), variant))
    result += MEOS_WKB_INT4_SIZE;
  /* Include the number of instants and the period bounds flag */
  result += MEOS_WKB_INT4_SIZE + MEOS_WKB_BYTE_SIZE;
  const TInstant **instants = tsequence_insts_p(seq);
  /* Include the TInstant array */
  result += tinstarr_to_wkb_size((TInstant **) instants, seq->count, variant);
  pfree(instants);
  return result;
}

/**
 * @brief Return the maximum size in bytes of the temporal sequence set in the
 * Well-Known Binary (WKB) representation
 */
static size_t
tsequenceset_to_wkb_size(const TSequenceSet *ss, uint8_t variant)
{
  /* Endian flag + temporal type + temporal flag */
  size_t result = MEOS_WKB_BYTE_SIZE * 2 + MEOS_WKB_INT2_SIZE;
  /* Extended WKB needs space for optional SRID integer */
  if (tspatial_type(ss->temptype) &&
      spatial_wkb_needs_srid(tspatial_srid((Temporal *) ss), variant))
    result += MEOS_WKB_INT4_SIZE;
  /* Include the number of sequences */
  result += MEOS_WKB_INT4_SIZE;
  /* For each sequence include the number of instants and the period bounds flag */
  result += ss->count * (MEOS_WKB_INT4_SIZE + MEOS_WKB_BYTE_SIZE);
  /* Include all the instants of all the sequences */
  const TInstant **instants = tsequenceset_insts_p(ss);
  result += tinstarr_to_wkb_size((TInstant **) instants, ss->totalcount,
    variant);
  pfree(instants);
  return result;
}

/**
 * @brief Return the maximum size in bytes of the temporal value in the
 * Well-Known Binary (WKB) representation
 */
static size_t
temporal_to_wkb_size(const Temporal *temp, uint8_t variant)
{
  size_t result = 0;
#if RGEO
  if (temp->temptype == T_TRGEOMETRY)
    result += geo_to_wkb_size(trgeo_geom_p(temp), variant);
#endif /* RGEO */

  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      result += tinstant_to_wkb_size((TInstant *) temp, variant);
      break;
    case TSEQUENCE:
      result += tsequence_to_wkb_size((TSequence *) temp, variant);
      break;
    default: /* TSEQUENCESET */
      result += tsequenceset_to_wkb_size((TSequenceSet *) temp, variant);
  }
  return result;
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
    return set_to_wkb_size(DatumGetSetP(value), variant);
  if (span_type(type))
    return span_to_wkb_size((Span *) DatumGetPointer(value), false);
  if (spanset_type(type))
    return spanset_to_wkb_size((SpanSet *) DatumGetPointer(value));
  if (type == T_TBOX)
    return tbox_to_wkb_size((TBox *) DatumGetPointer(value));
  if (type == T_STBOX)
    return stbox_to_wkb_size((STBox *) DatumGetPointer(value), variant);
#if CBUFFER
  if (type == T_CBUFFER)
    return cbuffer_to_wkb_size(DatumGetCbufferP(value), variant, false);
#endif /* CBUFFER */
#if NPOINT
  if (type == T_NPOINT)
    return npoint_to_wkb_size(DatumGetNpointP(value), variant, false);
#endif /* NPOINT */
#if POSE || RGEO
  if (type == T_POSE)
    return pose_to_wkb_size(DatumGetPoseP(value), variant, false);
#endif /* POSE || RGEO */
  if (temporal_type(type))
    return temporal_to_wkb_size((Temporal *) DatumGetPointer(value), variant);
  /* Error! */
  meos_error(ERROR, MEOS_ERR_WKB_OUTPUT,
    "Unknown type in WKB output: %s", meostype_name(type));
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
 * @brief Write into the buffer the Endian in the Well-Known Binary (WKB)
 * representation
 */
static uint8_t *
endian_to_wkb_buf(uint8_t *buf, uint8_t variant)
{
  if (variant & WKB_HEX)
  {
    buf[0] = '0';
    buf[1] = ((variant & WKB_NDR) ? '1' : '0');
    return buf + 2;
  }
  else
  {
    buf[0] = ((variant & WKB_NDR) ? 1 : 0);
    return buf + 1;
  }
}

/**
 * @brief Write into the buffer the bytes of the value in the Well-Known Binary
 * (WKB) representation
 */
static uint8_t *
bytes_to_wkb_buf(uint8_t *valptr, size_t size, uint8_t *buf, uint8_t variant)
{
  if (variant & WKB_HEX)
  {
    int swap = wkb_swap_bytes(variant);
    /* Machine/request arch mismatch, so flip byte order */
    for (size_t i = 0; i < size; i++)
    {
      int j = (int) (swap ? size - 1 - i : i);
      uint8_t b = valptr[j];
      /* Top four bits to 0-F */
      buf[2*i] = HEXCHR[b >> 4];
      /* Bottom four bits to 0-F */
      buf[2*i + 1] = HEXCHR[b & 0x0F];
    }
    return buf + (2 * size);
  }
  else
  {
    /* Machine/request arch mismatch, so flip byte order */
    if (wkb_swap_bytes(variant))
    {
      for (size_t i = 0; i < size; i++)
        buf[i] = valptr[size - 1 - i];
    }
    /* If machine arch and requested arch match, don't flip byte order */
    else
      memcpy(buf, valptr, size);
    return buf + size;
  }
}

/**
 * @brief Generic function to write a typed value to WKB buffer with size checking
 */
static inline uint8_t *
typed_value_to_wkb_buf(const void *value, size_t actual_size, size_t expected_size,
                       const char *type_name, uint8_t *buf, uint8_t variant)
{
  if (actual_size != expected_size)
  {
    meos_error(ERROR, MEOS_ERR_WKB_OUTPUT,
      "Machine %s size is not %d bytes!", type_name, (int)expected_size);
    return NULL;
  }
  uint8_t *valptr = (uint8_t *)(value);
  return bytes_to_wkb_buf(valptr, expected_size, buf, variant);
}

/**
 * @brief Write into the buffer a boolean in the Well-Known Binary (WKB)
 * representation
 */
static uint8_t *
bool_to_wkb_buf(bool b, uint8_t *buf, uint8_t variant)
{
  return typed_value_to_wkb_buf(&b, sizeof(bool), MEOS_WKB_BYTE_SIZE,
                                "bool", buf, variant);
}

/**
 * @brief Write into the buffer the int4 in the Well-Known Binary (WKB)
 * representation
 */
static uint8_t *
uint8_to_wkb_buf(const uint8_t i, uint8_t *buf, uint8_t variant)
{
  return typed_value_to_wkb_buf(&i, sizeof(int8), MEOS_WKB_BYTE_SIZE,
                                "int8", buf, variant);
}

/**
 * @brief Write into the buffer the int2 in the Well-Known Binary (WKB)
 * representation
 */
static uint8_t *
int16_to_wkb_buf(const int16 i, uint8_t *buf, uint8_t variant)
{
  return typed_value_to_wkb_buf(&i, sizeof(int16), MEOS_WKB_INT2_SIZE,
                                "int16", buf, variant);
}

/**
 * @brief Write into the buffer the int4 in the Well-Known Binary (WKB)
 * representation
 */
uint8_t *
int32_to_wkb_buf(const int i, uint8_t *buf, uint8_t variant)
{
  return typed_value_to_wkb_buf(&i, sizeof(int), MEOS_WKB_INT4_SIZE,
                                "int32", buf, variant);
}

/**
 * @brief Write into the buffer the int8 in the Well-Known Binary (WKB)
 * representation
 */
uint8_t *
int64_to_wkb_buf(const int64 i, uint8_t *buf, uint8_t variant)
{
  return typed_value_to_wkb_buf(&i, sizeof(int64), MEOS_WKB_INT8_SIZE,
                                "int64", buf, variant);
}

/**
 * @brief Write into the buffer the float64 in the Well-Known Binary (WKB)
 * representation
 */
uint8_t*
double_to_wkb_buf(const double d, uint8_t *buf, uint8_t variant)
{
  return typed_value_to_wkb_buf(&d, sizeof(double), MEOS_WKB_DOUBLE_SIZE,
                                "double", buf, variant);
}

/**
 * @brief Write into the buffer the TimestampTz (aka int64) in the Well-Known
 * Binary (WKB) representation
 */
uint8_t *
date_to_wkb_buf(const DateADT d, uint8_t *buf, uint8_t variant)
{
  return typed_value_to_wkb_buf(&d, sizeof(DateADT), MEOS_WKB_DATE_SIZE,
                                "date", buf, variant);
}

/**
 * @brief Write into the buffer the TimestampTz (aka int64) in the Well-Known
 * Binary (WKB) representation
 */
uint8_t *
timestamptz_to_wkb_buf(const TimestampTz t, uint8_t *buf, uint8_t variant)
{
  return typed_value_to_wkb_buf(&t, sizeof(TimestampTz), MEOS_WKB_TIMESTAMP_SIZE,
                                "timestamp", buf, variant);
}

/**
 * @brief Write into the buffer a text value in the Well-Known Binary (WKB)
 * representation
 */
static uint8_t *
text_to_wkb_buf(const text *txt, uint8_t *buf, uint8_t variant)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(txt, NULL);

  /*
   * Get the text data directly from the varlena structure.
   * This avoids the memory allocation of text_to_cstring.
   */
  size_t size = VARSIZE_ANY_EXHDR(txt);
  char *str = VARDATA(txt);

  /* Write the size first (this gets proper endian handling) */
  buf = int64_to_wkb_buf(size, buf, variant);

  /* Write the text data - needs special handling to avoid swapping */
  if (variant & WKB_HEX)
  {
    /* Convert to hex without swapping byte order */
    for (size_t i = 0; i < size; i++)
    {
      uint8_t b = str[i];
      /* Top four bits to 0-F */
      buf[2*i] = HEXCHR[b >> 4];
      /* Bottom four bits to 0-F */
      buf[2*i + 1] = HEXCHR[b & 0x0F];
    }
    return buf + (2 * size);
  }
  else
  {
    /* Binary - direct copy without swapping */
    memcpy(buf, str, size);
    return buf + size;
  }
}

/**
 * @brief Write into the buffer a geo value in the Well-Known Binary (WKB)
 * representation
 */
static uint8_t *
geo_to_wkb_buf(const GSERIALIZED *gs, uint8_t *buf, uint8_t variant)
{
  LWGEOM *geo = lwgeom_from_gserialized(gs);
  buf = lwgeom_to_wkb_buf(geo, buf, variant);
  lwgeom_free(geo);
  return buf;
}

#if CBUFFER
/**
 * @brief Write into the buffer a component circular buffer in the Well-Known 
 * Binary (WKB) representation
 * @details SRID (int32), coordinates of a 2D point and radius (3 doubles)
 */
static uint8_t *
cbuffer_to_wkb_buf(const Cbuffer *cb, uint8_t *buf, uint8_t variant,
  bool component)
{
  Datum d = PointerGetDatum(&cb->point);
  if (! component)
  {
    /* Write the endian flag (byte) */
    buf = endian_to_wkb_buf(buf, variant);
    /* Write the SRID flag */
    uint8_t wkb_flags = (uint8_t) MEOS_WKB_SRIDFLAG;
    buf = bytes_to_wkb_buf(&wkb_flags, MEOS_WKB_BYTE_SIZE, buf, variant);
    /* Write the SRID */
    int32_t srid = gserialized_get_srid(DatumGetGserializedP(d));
    if (spatial_wkb_needs_srid(srid, variant))
      buf = int32_to_wkb_buf(srid, buf, variant);
  }
  /* Write the circular buffer */
  const POINT2D *point = DATUM_POINT2D_P(d);
  buf = double_to_wkb_buf(point->x, buf, variant);
  buf = double_to_wkb_buf(point->y, buf, variant);
  buf = double_to_wkb_buf(cb->radius, buf, variant);
  return buf;
}
#endif /* CBUFFER */

#if NPOINT
/**
 * @brief Write into the buffer the flag of a network point in the Well-Known
 * Binary (WKB) representation
 * @details The output is a byte as follows
 * @code
 * xSGZxxTX
 * S = SRID, G = Geodetic, Z = has Z, T = has T, X = has X, x = unused bit
 * @endcode
 */
static uint8_t *
npoint_flags_to_wkb_buf(const Npoint *np, uint8_t *buf, uint8_t variant)
{
  uint8_t wkb_flags = 0;
  wkb_flags |= MEOS_WKB_XFLAG;
  if (spatial_wkb_needs_srid(npoint_srid(np), variant))
    wkb_flags |= MEOS_WKB_SRIDFLAG;
  /* Write the flags */
  return uint8_to_wkb_buf(wkb_flags, buf, variant);
}

/**
 * @brief Write into the buffer a network point in the Well-Known Binary
 * (WKB) representation
 */
static uint8_t *
npoint_to_wkb_buf(const Npoint *np, uint8_t *buf, uint8_t variant,
  bool component)
{
  if (! component)
    /* Write the endian flag (byte) */
    buf = endian_to_wkb_buf(buf, variant);
  /* Write the flags (byte) */
  buf = npoint_flags_to_wkb_buf(np, buf, variant);
  /* Write the SRID */
  int32_t srid = npoint_srid(np);
  if (spatial_wkb_needs_srid(srid, variant))
    buf = int32_to_wkb_buf(srid, buf, variant);
  /* Write the network point */
  buf = int64_to_wkb_buf(np->rid, buf, variant);
  buf = double_to_wkb_buf(np->pos, buf, variant);
  return buf;
}
#endif /* NPOINT */

#if POSE || RGEO
/**
 * @brief Write into the buffer the flag of a pose in the Well-Known Binary
 * (WKB) representation
 * @details The output is a byte as follows
 * @code
 * xSGZxxTX
 * S = SRID, G = Geodetic, Z = has Z, T = has T, X = has X, x = unused bit
 * @endcode
 */
static uint8_t *
pose_flags_to_wkb_buf(const Pose *pose, uint8_t *buf, uint8_t variant)
{
  uint8_t wkb_flags = 0;
  wkb_flags |= MEOS_WKB_XFLAG;
  if (MEOS_FLAGS_GET_Z(pose->flags))
    wkb_flags |= MEOS_WKB_ZFLAG;
  if (spatial_wkb_needs_srid(pose_srid(pose), variant))
    wkb_flags |= MEOS_WKB_SRIDFLAG;
  /* Write the flags */
  return uint8_to_wkb_buf(wkb_flags, buf, variant);
}

/**
 * @brief Write into the buffer a component pose in the Well-Known Binary (WKB)
 * representation
 * @details SRID (int32, if any), 2D: 3 doubles, 3D: 7 doubles
 */
static uint8_t *
pose_to_wkb_buf(const Pose *pose, uint8_t *buf, uint8_t variant,
  bool component)
{
  if (! component)
    /* Write the endian flag (byte) */
    buf = endian_to_wkb_buf(buf, variant);
  /* Write the flags (byte) */
  buf = pose_flags_to_wkb_buf(pose, buf, variant);
  int32_t srid = pose_srid(pose);
  if (spatial_wkb_needs_srid(srid, variant))
    buf = int32_to_wkb_buf(srid, buf, variant);
  /* Write the pose */
  buf = double_to_wkb_buf(pose->data[0], buf, variant);
  buf = double_to_wkb_buf(pose->data[1], buf, variant);
  buf = double_to_wkb_buf(pose->data[2], buf, variant);
  if (MEOS_FLAGS_GET_Z(pose->flags))
  {
    buf = double_to_wkb_buf(pose->data[3], buf, variant);
    buf = double_to_wkb_buf(pose->data[4], buf, variant);
    buf = double_to_wkb_buf(pose->data[5], buf, variant);
    buf = double_to_wkb_buf(pose->data[6], buf, variant);
  }
  return buf;
}
#endif /* POSE */

/**
 * @brief Write into the buffer a temporal instant in the Well-Known Binary
 * (WKB) representation
 * @details The output is as follows
 * - base value
 * - timestamp
 */
static uint8_t *
base_to_wkb_buf(Datum value, meosType basetype, uint8_t *buf,
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
      buf = geo_to_wkb_buf(DatumGetGserializedP(value), buf, variant);
      break;
#if CBUFFER
    case T_CBUFFER:
      buf = cbuffer_to_wkb_buf(DatumGetCbufferP(value), buf, variant, true);
      break;
#endif /* CBUFFER */
#if NPOINT
    case T_NPOINT:
      buf = npoint_to_wkb_buf(DatumGetNpointP(value), buf, variant, true);
      break;
#endif /* NPOINT */
#if POSE || RGEO
    case T_POSE:
      buf = pose_to_wkb_buf(DatumGetPoseP(value), buf, variant, true);
      break;
#endif /* POSE || RGEO */
    default: /* Error! */
      meos_error(ERROR, MEOS_ERR_WKB_OUTPUT,
        "Unknown basetype in WKB output: %s", meostype_name(basetype));
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
  if (spatial_basetype(set->basetype))
  {
    if (MEOS_FLAGS_GET_Z(set->flags))
      wkb_flags |= MEOS_WKB_ZFLAG;
    if (MEOS_FLAGS_GET_GEODETIC(set->flags))
      wkb_flags |= MEOS_WKB_GEODETICFLAG;
    if (spatialset_type(set->settype) &&
        spatial_wkb_needs_srid(spatialset_srid(set), variant))
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
  /* Write the endian flag (byte) */
  buf = endian_to_wkb_buf(buf, variant);
  /* Write the set type */
  buf = int16_to_wkb_buf(set->settype, buf, variant);
  /* Write the set flags (byte) */
  buf = set_flags_to_wkb_buf(set, buf, variant);
  /* Write the optional SRID for extended variant */
  if (spatialset_type(set->settype) &&
      spatial_wkb_needs_srid(spatialset_srid(set), variant))
    buf = int32_to_wkb_buf(spatialset_srid(set), buf, variant);
  /* Write the count */
  buf = int32_to_wkb_buf(set->count, buf, variant);
  /* Write the values */
  for (int i = 0; i < set->count; i++)
    buf = base_to_wkb_buf(SET_VAL_N(set, i), set->basetype, buf, variant);
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
        "Unknown span base type in WKB output: %s",
        meostype_name(s->basetype));
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
span_to_wkb_buf_iter(const Span *s, uint8_t *buf, uint8_t variant)
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
 * - Endian byte (if not a component)
 * - Basetype @p int16
 * - Bounds byte stating whether the bounds are inclusive
 * - Two base type values
 */
uint8_t *
span_to_wkb_buf(const Span *s, uint8_t *buf, uint8_t variant, bool component)
{
  if (! component)
    /* Write the endian flag (byte) */
    buf = endian_to_wkb_buf(buf, variant);
  /* Write the span type */
  buf = int16_to_wkb_buf(s->spantype, buf, variant);
  /* Write the span bounds and values */
  buf = span_to_wkb_buf_iter(s, buf, variant);
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
  /* Write the endian flag (byte) */
  buf = endian_to_wkb_buf(buf, variant);
  /* Write the span type */
  buf = int16_to_wkb_buf(ss->spansettype, buf, variant);
  /* Write the count */
  buf = int32_to_wkb_buf(ss->count, buf, variant);
  /* Write the periods */
  for (int i = 0; i < ss->count; i++)
    buf = span_to_wkb_buf_iter(SPANSET_SP_N(ss, i), buf, variant);
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
  /* Write the endian flag (byte) */
  buf = endian_to_wkb_buf(buf, variant);
  /* Write the temporal flags */
  buf = tbox_to_wkb_flags_buf(box, buf, variant);
  /* Write the temporal dimension if any */
  if (MEOS_FLAGS_GET_T(box->flags))
    buf = span_to_wkb_buf(&box->period, buf, variant, true);
  /* Write the value dimension if any */
  if (MEOS_FLAGS_GET_X(box->flags))
    buf = span_to_wkb_buf(&box->span, buf, variant, true);
  return buf;
}

/*****************************************************************************/

/**
 * @brief Write into the buffer the flag of a spatiotemporal box in the
 * Well-Known Binary (WKB) representation
 * @details The output is a byte as follows
 * @code
 * xSGZxxTX
 * S = SRID, G = Geodetic, Z = has Z, T = has T, X = has X, x = unused bit
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
  if (spatial_wkb_needs_srid(box->srid, variant))
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
  /* Write the endian flag (byte) */
  buf = endian_to_wkb_buf(buf, variant);
  /* Write the temporal flags */
  buf = stbox_flags_to_wkb_buf(box, buf, variant);
  /* Write the optional SRID for extended variant */
  if (spatial_wkb_needs_srid(box->srid, variant))
    buf = int32_to_wkb_buf(stbox_srid(box), buf, variant);
  /* Write the temporal dimension if any */
  if (MEOS_FLAGS_GET_T(box->flags))
    buf = span_to_wkb_buf(&box->period, buf, variant, true);
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
  if (tspatial_type(temp->temptype))
  {
    if (MEOS_FLAGS_GET_Z(temp->flags))
      wkb_flags |= MEOS_WKB_ZFLAG;
    if (MEOS_FLAGS_GET_GEODETIC(temp->flags))
      wkb_flags |= MEOS_WKB_GEODETICFLAG;
    if (spatial_wkb_needs_srid(tspatial_srid(temp), variant))
      wkb_flags |= MEOS_WKB_SRIDFLAG;
  }
  /* Write the flags */
  return bytes_to_wkb_buf(&wkb_flags, MEOS_WKB_BYTE_SIZE, buf, variant);
}

/**
 * @brief Write into the buffer a temporal instant in the Well-Known Binary
 * (WKB) representation
 * @details The output is as follows
 * - base value
 * - timestamp
 */
static uint8_t *
tinstant_base_time_to_wkb_buf(const TInstant *inst, uint8_t *buf,
  uint8_t variant)
{
  meosType basetype = temptype_basetype(inst->temptype);
  assert(temporal_basetype(basetype));
  buf = base_to_wkb_buf(tinstant_value_p(inst), basetype, buf, variant);
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
  /* Write the endian flag (byte) */
  buf = endian_to_wkb_buf(buf, variant);
  /* Write the temporal type */
  buf = int16_to_wkb_buf(inst->temptype, buf, variant);
  /* Write the temporal flags */
  buf = temporal_flags_to_wkb_buf((Temporal *) inst, buf, variant);
  /* Write the optional SRID for extended variant */
  if (tspatial_type(inst->temptype))
  {
    int32_t srid = tspatial_srid((Temporal *) inst);
    if (spatial_wkb_needs_srid(srid, variant))
      buf = int32_to_wkb_buf(srid, buf, variant);
  }
#if RGEO
  if (inst->temptype == T_TRGEOMETRY)
    buf = geo_to_wkb_buf(trgeoinst_geom_p(inst), buf, variant);
#endif /* RGEO */
  return tinstant_base_time_to_wkb_buf(inst, buf, variant);
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
  /* Write the endian flag (byte) */
  buf = endian_to_wkb_buf(buf, variant);
  /* Write the temporal type */
  buf = int16_to_wkb_buf(seq->temptype, buf, variant);
  /* Write the temporal and interpolation flags */
  buf = temporal_flags_to_wkb_buf((Temporal *) seq, buf, variant);
  /* Write the optional SRID for extended variant */
  if (tspatial_type(seq->temptype) &&
      spatial_wkb_needs_srid(tspatial_srid((Temporal *) seq), variant))
    buf = int32_to_wkb_buf(tspatial_srid((Temporal *) seq), buf, variant);
#if RGEO
  if (seq->temptype == T_TRGEOMETRY)
    buf = geo_to_wkb_buf(trgeoseq_geom_p(seq), buf, variant);
#endif /* RGEO */
  /* Write the count */
  buf = int32_to_wkb_buf(seq->count, buf, variant);
  /* Write the period bounds */
  buf = bounds_to_wkb_buf(seq->period.lower_inc, seq->period.upper_inc, buf,
    variant);
  /* Write the array of instants */
  for (int i = 0; i < seq->count; i++)
    buf = tinstant_base_time_to_wkb_buf(TSEQUENCE_INST_N(seq, i), buf,
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
  /* Write the endian flag (byte) */
  buf = endian_to_wkb_buf(buf, variant);
  /* Write the temporal type */
  buf = int16_to_wkb_buf(ss->temptype, buf, variant);
  /* Write the temporal and interpolation flags */
  buf = temporal_flags_to_wkb_buf((Temporal *) ss, buf, variant);
  /* Write the optional SRID for extended variant */
  if (tspatial_type(ss->temptype) &&
      spatial_wkb_needs_srid(tspatial_srid((Temporal *) ss), variant))
    buf = int32_to_wkb_buf(tspatial_srid((Temporal *) ss), buf, variant);
#if RGEO
  if (ss->temptype == T_TRGEOMETRY)
    buf = geo_to_wkb_buf(trgeoseqset_geom_p(ss), buf, variant);
#endif /* RGEO */
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
      buf = tinstant_base_time_to_wkb_buf(TSEQUENCE_INST_N(seq, j), buf,
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
    buf = set_to_wkb_buf(DatumGetSetP(value), buf, variant);
  else if (span_type(type))
    buf = span_to_wkb_buf(DatumGetSpanP(value), buf, variant, false);
  else if (spanset_type(type))
    buf = spanset_to_wkb_buf(DatumGetSpanSetP(value), buf, variant);
  else if (type == T_TBOX)
    buf = tbox_to_wkb_buf(DatumGetTboxP(value), buf, variant);
  else if (type == T_STBOX)
    buf = stbox_to_wkb_buf(DatumGetSTboxP(value), buf, variant);
#if CBUFFER
  else if (type == T_CBUFFER)
    buf = cbuffer_to_wkb_buf(DatumGetCbufferP(value), buf, variant, false);
#endif /* CBUFFER */
#if NPOINT
  else if (type == T_NPOINT)
    buf = npoint_to_wkb_buf(DatumGetNpointP(value), buf, variant,
      false);
#endif /* NPOINT */
#if POSE || RGEO
  else if (type == T_POSE)
    buf = pose_to_wkb_buf(DatumGetPoseP(value), buf, variant, false);
#endif /* POSE || RGEO */
  else if (temporal_type(type))
    buf = temporal_to_wkb_buf((Temporal *) DatumGetPointer(value), buf,
      variant);
  else /* Error! */
  {
    meos_error(ERROR, MEOS_ERR_WKB_OUTPUT,
      "Unknown type in WKB output: %s", meostype_name(type));
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
 * WKB form as ASCII hex-encoded.
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
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(s, NULL); VALIDATE_NOT_NULL(size_out, NULL);
  return datum_as_wkb(PointerGetDatum(s), s->settype, variant, size_out);
}

#if MEOS
/**
 * @ingroup meos_setspan_inout
 * @brief Return the ASCII hex-encoded Well-Known Binary (HexWKB)
 * representation of a set
 * @param[in] s Set
 * @param[in] variant Output variant
 * @param[out] size_out Size of the output
 * @csqlfn #Set_as_hexwkb()
 */
char *
set_as_hexwkb(const Set *s, uint8_t variant, size_t *size_out)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(s, NULL); VALIDATE_NOT_NULL(size_out, NULL);
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
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(s, NULL); VALIDATE_NOT_NULL(size_out, NULL);
  return datum_as_wkb(PointerGetDatum(s), s->spantype, variant, size_out);
}

#if MEOS
/**
 * @ingroup meos_setspan_inout
 * @brief Return the ASCII hex-encoded Well-Known Binary (HexWKB)
 * representation of a span
 * @param[in] s Span
 * @param[in] variant Output variant
 * @param[out] size_out Size of the output
 * @csqlfn #Span_as_hexwkb()
 */
char *
span_as_hexwkb(const Span *s, uint8_t variant, size_t *size_out)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(s, NULL); VALIDATE_NOT_NULL(size_out, NULL);
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
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(ss, NULL); VALIDATE_NOT_NULL(size_out, NULL);
  return datum_as_wkb(PointerGetDatum(ss), ss->spansettype, variant, size_out);
}

#if MEOS
/**
 * @ingroup meos_setspan_inout
 * @brief Return the ASCII hex-encoded Well-Known Binary (HexWKB)
 * representation of a span set
 * @param[in] ss Span set
 * @param[in] variant Output variant
 * @param[out] size_out Size of the output
 * @csqlfn #Spanset_as_hexwkb()
 */
char *
spanset_as_hexwkb(const SpanSet *ss, uint8_t variant, size_t *size_out)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(ss, NULL); VALIDATE_NOT_NULL(size_out, NULL);
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
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, NULL); VALIDATE_NOT_NULL(size_out, NULL);
  return datum_as_wkb(PointerGetDatum(box), T_TBOX, variant,
    size_out);
}

#if MEOS
/**
 * @ingroup meos_box_inout
 * @brief Return the ASCII hex-encoded Well-Known Binary (HexWKB)
 * representation of a temporal box
 * @param[in] box Temporal box
 * @param[in] variant Output variant
 * @param[out] size_out Size of the output
 * @csqlfn #Tbox_as_hexwkb()
 */
char *
tbox_as_hexwkb(const TBox *box, uint8_t variant, size_t *size_out)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, NULL); VALIDATE_NOT_NULL(size_out, NULL);
  return (char *) datum_as_wkb(PointerGetDatum(box), T_TBOX,
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
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL); VALIDATE_NOT_NULL(size_out, NULL);
  return datum_as_wkb(PointerGetDatum(temp), temp->temptype, variant,
    size_out);
}

#if MEOS
/**
 * @ingroup meos_temporal_inout
 * @brief Return the ASCII hex-encoded Well-Known Binary (HexWKB)
 * representation of a temporal value
 * @param[in] temp Temporal value
 * @param[in] variant Output variant
 * @param[out] size_out Size of the output
 * @csqlfn #Temporal_as_hexwkb()
 */
char *
temporal_as_hexwkb(const Temporal *temp, uint8_t variant, size_t *size_out)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(temp, NULL); VALIDATE_NOT_NULL(size_out, NULL);
  return (char *) datum_as_wkb(PointerGetDatum(temp), temp->temptype,
    variant | (uint8_t) WKB_HEX, size_out);
}
#endif /* MEOS */

/*****************************************************************************/
