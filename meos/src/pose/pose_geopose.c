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
 * @brief OGC GeoPose JSON I/O — Basic-YPR and Basic-Quaternion conformance.
 *
 * Implements the OGC GeoPose v1.0 standard's two Basic conformance classes:
 *
 *   {                                  {
 *     "position": {                      "position": {
 *       "lat": 47.0,                       "lat": 47.0,
 *       "lon": 8.0,                        "lon": 8.0,
 *       "h":   1500.0                      "h":   1500.0
 *     },                                 },
 *     "quaternion": {                    "angles": {
 *       "x": 0.0,                          "yaw":   90.0,
 *       "y": 0.0,                          "pitch":  0.0,
 *       "z": 0.7071,                       "roll":   0.0
 *       "w": 0.7071                      }
 *     }                                }
 *   }
 *
 * Position is geographic (lat / lon in degrees, height h in metres). The
 * standard mandates a geographic outer frame for the Basic classes; this
 * implementation accepts only SRID 4326 inputs (or unknown SRID, treated as
 * geographic) and rejects projected SRIDs with a clear error.
 *
 * On input the parser auto-detects the conformance class from the keys
 * present (`quaternion` vs `angles`). On output the caller picks the class
 * via `GeoPoseClass`; the default chosen by the SQL surface is
 * Basic-Quaternion since it is lossless for our internal representation.
 *
 * 2D poses (data = [x, y, theta]) are represented in JSON with `h: 0`,
 * `pitch: 0`, `roll: 0`, and yaw = theta. `theta` is stored in radians
 * internally; the JSON form uses degrees, per the standard.
 *
 * The Advanced GeoPose class (frame stacks, covariance) is not implemented.
 */

/* C */
#include <math.h>
/* json-c */
#include <json-c/json.h>
/* PostgreSQL */
#include <postgres.h>
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif
/* MEOS */
#include <meos.h>
#include <meos_pose.h>
#include <meos_internal.h>
#include "temporal/temporal.h"      /* ensure_not_null */
#include "pose/pose.h"
#include "pose/pose_geopose.h"

/*****************************************************************************
 * Helpers
 *****************************************************************************/

/* OGC GeoPose Basic conformance classes assume a geographic outer frame.
 * SRID 4326 (WGS-84) is the only one we currently expose; SRID 0 is
 * accepted on input and treated as geographic. */
#define GEOPOSE_GEOGRAPHIC_SRID 4326

/**
 * @brief Look up a JSON object member case-insensitively.
 * @details Locally duplicated from the equivalent helper in
 * `meos/src/temporal/type_in.c` (which is `static`) so this module
 * doesn't need to lift that helper into a public header.
 */
static json_object *
geopose_find_member(json_object *obj, const char *name)
{
  json_object *tmp = obj;
  json_object_iter it;
  if (name == NULL || obj == NULL)
    return NULL;
  it.key = NULL; it.val = NULL; it.entry = NULL;
  if (! json_object_get_object(tmp))
    return NULL;
  if (! json_object_get_object(tmp)->head)
    return NULL;
  for (it.entry = json_object_get_object(tmp)->head;
      ( it.entry ?
        ( it.key = (char *) it.entry->k,
          it.val = (json_object *) it.entry->v, it.entry) : 0);
      it.entry = it.entry->next)
  {
    if (pg_strcasecmp(it.key, name) == 0)
      return it.val;
  }
  return NULL;
}

/**
 * @brief Read a numeric JSON member (`int` or `double`); set @p found
 * if the member existed and `*out` was filled.
 */
static bool
geopose_get_number(json_object *obj, const char *name, double *out)
{
  json_object *jv = geopose_find_member(obj, name);
  if (jv == NULL)
    return false;
  if (json_object_is_type(jv, json_type_int))
    *out = (double) json_object_get_int64(jv);
  else if (json_object_is_type(jv, json_type_double))
    *out = json_object_get_double(jv);
  else
    return false;
  return true;
}

/**
 * @brief Convert (yaw, pitch, roll) — radians, ZYX intrinsic Tait-Bryan
 * convention — to a unit quaternion (W, X, Y, Z) in Hamilton convention.
 * @details ZYX order matches the OGC GeoPose spec (yaw about Z, then
 * pitch about new Y, then roll about new X). The output is unit-norm by
 * construction (modulo float rounding).
 */
static void
geopose_ypr_to_quaternion(double yaw_rad, double pitch_rad, double roll_rad,
  double *W, double *X, double *Y, double *Z)
{
  double cy = cos(yaw_rad   * 0.5), sy = sin(yaw_rad   * 0.5);
  double cp = cos(pitch_rad * 0.5), sp = sin(pitch_rad * 0.5);
  double cr = cos(roll_rad  * 0.5), sr = sin(roll_rad  * 0.5);
  *W = cr * cp * cy + sr * sp * sy;
  *X = sr * cp * cy - cr * sp * sy;
  *Y = cr * sp * cy + sr * cp * sy;
  *Z = cr * cp * sy - sr * sp * cy;
}

/**
 * @brief Convert a unit quaternion (W, X, Y, Z, Hamilton convention) to
 * (yaw, pitch, roll) in radians, ZYX intrinsic Tait-Bryan convention.
 * @details The pitch term is clamped to [-1, 1] before `asin` to absorb
 * the small numeric drift `|q| - 1 = O(1e-15)` that long quaternion
 * compositions can introduce.
 */
static void
geopose_quaternion_to_ypr(double W, double X, double Y, double Z,
  double *yaw_rad, double *pitch_rad, double *roll_rad)
{
  double sinp = 2.0 * (W * Y - Z * X);
  if (sinp > 1.0)  sinp = 1.0;
  if (sinp < -1.0) sinp = -1.0;
  *pitch_rad = asin(sinp);
  *roll_rad  = atan2(2.0 * (W * X + Y * Z),
                     1.0 - 2.0 * (X * X + Y * Y));
  *yaw_rad   = atan2(2.0 * (W * Z + X * Y),
                     1.0 - 2.0 * (Y * Y + Z * Z));
}

#define GEOPOSE_DEG2RAD(d) ((d) * (M_PI / 180.0))
#define GEOPOSE_RAD2DEG(r) ((r) * (180.0 / M_PI))

/**
 * @brief Build a JSON double with a caller-controlled precision.
 * @details Uses `json_object_new_double_s` so the serializer emits the
 * pre-formatted representation rather than json-c's default 17-digit
 * lossless form. A negative @p precision keeps the default.
 */
static json_object *
geopose_new_double(double v, int precision)
{
  if (precision < 0)
    return json_object_new_double(v);
  char buf[64];
  /* `%.*g` keeps `precision` significant digits — same convention
   * MobilityDB uses elsewhere (e.g., `tspatial_as_text(temp, 6)`). */
  snprintf(buf, sizeof(buf), "%.*g", precision, v);
  return json_object_new_double_s(v, buf);
}

/*****************************************************************************
 * Input — internal helper that parses a single GeoPose object node
 *****************************************************************************/

/**
 * @brief Build a pose from a parsed Basic-class GeoPose JSON object.
 * @details Used internally by @p pose_from_geopose (single-pose entry
 * point) and by the temporal-GeoPose entry points which iterate an
 * envelope's @p instants array. The caller owns the @p root and is
 * responsible for releasing it.
 */
static Pose *
pose_from_geopose_object(json_object *root)
{
  /* Position: {lat, lon, h} */
  json_object *jpos = geopose_find_member(root, "position");
  if (jpos == NULL || ! json_object_is_type(jpos, json_type_object))
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "GeoPose JSON missing required 'position' object");
    return NULL;
  }
  double lat, lon, h = 0.0;
  bool have_h = geopose_get_number(jpos, "h", &h);
  if (! geopose_get_number(jpos, "lat", &lat) ||
      ! geopose_get_number(jpos, "lon", &lon))
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "GeoPose 'position' must carry numeric 'lat' and 'lon' members");
    return NULL;
  }

  /* Orientation: detect class. */
  json_object *jq  = geopose_find_member(root, "quaternion");
  json_object *jyp = geopose_find_member(root, "angles");
  Pose *result = NULL;

  if (jq != NULL && json_object_is_type(jq, json_type_object))
  {
    /* Basic-Quaternion */
    double W, X, Y, Z;
    if (! geopose_get_number(jq, "w", &W) ||
        ! geopose_get_number(jq, "x", &X) ||
        ! geopose_get_number(jq, "y", &Y) ||
        ! geopose_get_number(jq, "z", &Z))
    {
      meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
        "GeoPose 'quaternion' must carry numeric 'w','x','y','z' members");
      return NULL;
    }
    /* Note: any reasonable client emits a unit quaternion. We keep the
     * input verbatim — the canonicalisation/normalisation pass is a
     * separate phase. */
    result = pose_make_3d(lon, lat, h, W, X, Y, Z, GEOPOSE_GEOGRAPHIC_SRID);
  }
  else if (jyp != NULL && json_object_is_type(jyp, json_type_object))
  {
    /* Basic-YPR */
    double yaw_deg = 0.0, pitch_deg = 0.0, roll_deg = 0.0;
    bool have_yaw   = geopose_get_number(jyp, "yaw",   &yaw_deg);
    bool have_pitch = geopose_get_number(jyp, "pitch", &pitch_deg);
    bool have_roll  = geopose_get_number(jyp, "roll",  &roll_deg);
    if (! have_yaw && ! have_pitch && ! have_roll)
    {
      meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
        "GeoPose 'angles' must carry at least one of "
        "'yaw','pitch','roll' (numeric, in degrees)");
      return NULL;
    }
    /* 2D pose iff pitch=roll=0 and h is absent (or zero) and the only
     * rotation is about the vertical axis. This matches the convention
     * for terrestrial trajectories where height and tilt are not
     * tracked. */
    if (! have_h && pitch_deg == 0.0 && roll_deg == 0.0)
    {
      double theta_rad = GEOPOSE_DEG2RAD(yaw_deg);
      result = pose_make_2d(lon, lat, theta_rad, GEOPOSE_GEOGRAPHIC_SRID);
    }
    else
    {
      double W, X, Y, Z;
      geopose_ypr_to_quaternion(GEOPOSE_DEG2RAD(yaw_deg),
        GEOPOSE_DEG2RAD(pitch_deg), GEOPOSE_DEG2RAD(roll_deg),
        &W, &X, &Y, &Z);
      result = pose_make_3d(lon, lat, h, W, X, Y, Z,
        GEOPOSE_GEOGRAPHIC_SRID);
    }
  }
  else
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "GeoPose JSON requires either a 'quaternion' object "
      "(Basic-Quaternion) or an 'angles' object (Basic-YPR)");
    return NULL;
  }

  return result;
}

/**
 * @ingroup meos_pose_base_geopose
 * @brief Return a pose from its OGC GeoPose JSON representation
 * @param[in] json GeoPose JSON string (Basic-YPR or Basic-Quaternion)
 * @return On error return @p NULL
 * @details Auto-detects the conformance class from the JSON keys present:
 *
 *   - `quaternion`: Basic-Quaternion (returns a 3D pose).
 *   - `angles`:     Basic-YPR (returns a 3D pose; or 2D if pitch=roll=0
 *     and h=0, since that is the canonical 2D representation).
 *
 * Position is parsed as `{lat, lon, h}` (degrees, degrees, metres). The
 * resulting pose has SRID 4326 (WGS-84). Projected SRIDs are not
 * supported by this entry point — use the WKT/WKB I/O for those.
 */
Pose *
pose_from_geopose(const char *json)
{
  if (json == NULL)
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "Null GeoPose JSON string");
    return NULL;
  }

  json_tokener *tok = json_tokener_new();
  json_object *root = json_tokener_parse_ex(tok, json, -1);
  if (tok->err != json_tokener_success || root == NULL)
  {
    char err[256];
    snprintf(err, sizeof(err), "%s (at offset %d)",
      json_tokener_error_desc(tok->err), tok->char_offset);
    json_tokener_free(tok);
    if (root) json_object_put(root);
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "Invalid GeoPose JSON: %s", err);
    return NULL;
  }
  json_tokener_free(tok);

  Pose *result = pose_from_geopose_object(root);
  json_object_put(root);
  return result;
}

/*****************************************************************************
 * Output — internal helper that builds a single GeoPose object node
 *****************************************************************************/

/**
 * @brief Build the Basic-class GeoPose JSON object for a single pose.
 * @details Used internally by @p pose_as_geopose (single-pose entry
 * point) and by the temporal-GeoPose entry points which embed the
 * per-instant object into an envelope's @p instants array. The caller
 * owns the returned object and is responsible for releasing it.
 * @return On error return @p NULL.
 */
static json_object *
pose_to_geopose_object(const Pose *pose, int conformance, int precision)
{
  if (conformance != GEOPOSE_BASIC_QUATERNION &&
      conformance != GEOPOSE_BASIC_YPR)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Unknown GeoPose conformance class %d "
      "(0 = Basic-Quaternion, 1 = Basic-YPR)", conformance);
    return NULL;
  }
  int32_t srid = pose_srid(pose);
  if (srid != 0 && srid != GEOPOSE_GEOGRAPHIC_SRID)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "GeoPose JSON requires SRID 4326 (WGS-84), got %d", srid);
    return NULL;
  }
  bool has_z = MEOS_FLAGS_GET_Z(pose->flags);
  /* Position: x is longitude, y is latitude, z (if present) is height. */
  double lon = pose->data[0];
  double lat = pose->data[1];
  double h   = has_z ? pose->data[2] : 0.0;

  /* Orientation. Both classes start from a quaternion: read it from a
   * 3D pose, or build it from the 2D theta. */
  double W, X, Y, Z;
  if (has_z)
  {
    W = pose->data[3]; X = pose->data[4];
    Y = pose->data[5]; Z = pose->data[6];
  }
  else
  {
    /* 2D pose: pure yaw rotation about Z. */
    double theta_rad = pose->data[2];
    geopose_ypr_to_quaternion(theta_rad, 0.0, 0.0, &W, &X, &Y, &Z);
  }

  json_object *root = json_object_new_object();
  json_object *jpos = json_object_new_object();
  json_object_object_add(jpos, "lat", geopose_new_double(lat, precision));
  json_object_object_add(jpos, "lon", geopose_new_double(lon, precision));
  /* Always emit `h` even for 2D poses so the JSON document is a
   * complete Basic-class instance. */
  json_object_object_add(jpos, "h",   geopose_new_double(h, precision));
  json_object_object_add(root, "position", jpos);

  if (conformance == GEOPOSE_BASIC_QUATERNION)
  {
    json_object *jq = json_object_new_object();
    json_object_object_add(jq, "x", geopose_new_double(X, precision));
    json_object_object_add(jq, "y", geopose_new_double(Y, precision));
    json_object_object_add(jq, "z", geopose_new_double(Z, precision));
    json_object_object_add(jq, "w", geopose_new_double(W, precision));
    json_object_object_add(root, "quaternion", jq);
  }
  else /* GEOPOSE_BASIC_YPR */
  {
    double yaw_rad, pitch_rad, roll_rad;
    geopose_quaternion_to_ypr(W, X, Y, Z, &yaw_rad, &pitch_rad, &roll_rad);
    json_object *ja = json_object_new_object();
    json_object_object_add(ja, "yaw",
      geopose_new_double(GEOPOSE_RAD2DEG(yaw_rad), precision));
    json_object_object_add(ja, "pitch",
      geopose_new_double(GEOPOSE_RAD2DEG(pitch_rad), precision));
    json_object_object_add(ja, "roll",
      geopose_new_double(GEOPOSE_RAD2DEG(roll_rad), precision));
    json_object_object_add(root, "angles", ja);
  }
  return root;
}

/**
 * @ingroup meos_pose_base_geopose
 * @brief Return the OGC GeoPose JSON representation of a pose
 * @param[in] pose Pose value
 * @param[in] conformance Conformance class to emit (Basic-Quaternion or Basic-YPR)
 * @param[in] precision Decimal places to keep in the JSON numbers; pass
 * a negative value to use json-c's default
 * @return On error return @p NULL
 * @details The Basic conformance classes mandate a geographic outer
 * frame. Poses with SRID 0 are treated as geographic; poses with
 * non-zero non-4326 SRIDs are rejected (use WKT/WKB instead).
 */
char *
pose_as_geopose(const Pose *pose, int conformance, int precision)
{
  VALIDATE_NOT_NULL(pose, NULL);
  json_object *root = pose_to_geopose_object(pose, conformance, precision);
  if (root == NULL)
    return NULL;

  int flags = JSON_C_TO_STRING_PLAIN;
  const char *raw = json_object_to_json_string_ext(root, flags);
  char *out = pstrdup(raw);
  json_object_put(root);
  return out;
}

/*****************************************************************************
 * Temporal-GeoPose I/O
 *
 * The temporal envelope wraps an array of instants whose payloads are
 * each a strictly OGC-valid Basic-class GeoPose object (with an added
 * `validTime` member). A non-MEOS GeoPose consumer can iterate
 * `instants[]` (or each `sequences[].instants[]` for a sequence set)
 * and consume each element as a static GeoPose document.
 *
 * Envelope shape:
 *   {
 *     "type":          "TemporalGeoPose",
 *     "version":       "1.0",
 *     "conformance":   "Basic-Quaternion" | "Basic-YPR",
 *     "interpolation": "Discrete" | "Step" | "Linear",
 *     "instants":      [...]                  // for TInstant + TSequence
 *     "lower_inc":     true|false,            // for TSequence only
 *     "upper_inc":     true|false,            // for TSequence only
 *     "sequences":     [{...}, ...]           // for TSequenceSet only
 *   }
 *
 * The per-instant object adds `validTime` to the static Basic-class
 * shape, e.g.:
 *   {"validTime":"2026-01-01T00:00:00+00",
 *    "position":{"lat":47,"lon":8,"h":1500},
 *    "quaternion":{"x":0,"y":0,"z":0,"w":1}}
 *****************************************************************************/

/* String literals for the envelope's `interpolation` and `conformance` */
static const char *
geopose_interp_name(interpType interp)
{
  switch (interp)
  {
    case DISCRETE: return "Discrete";
    case STEP:     return "Step";
    case LINEAR:   return "Linear";
    default:       return "None";
  }
}

static interpType
geopose_interp_from_string(const char *str)
{
  if (str == NULL) return INTERP_NONE;
  if (strcmp(str, "Discrete") == 0) return DISCRETE;
  if (strcmp(str, "Step")     == 0) return STEP;
  if (strcmp(str, "Linear")   == 0) return LINEAR;
  return INTERP_NONE;
}

static const char *
geopose_conformance_name(int conformance)
{
  return (conformance == GEOPOSE_BASIC_YPR) ? "Basic-YPR" : "Basic-Quaternion";
}

/**
 * @brief Append the per-instant Basic-class object (with `validTime`) to
 * @p instants_array. Returns false on error.
 */
static bool
tposeinst_append_geopose(json_object *instants_array, const TInstant *inst,
  int conformance, int precision)
{
  const Pose *pose = DatumGetPoseP(tinstant_value_p(inst));
  json_object *obj = pose_to_geopose_object(pose, conformance, precision);
  if (obj == NULL)
    return false;
  char *vt = pg_timestamptz_out(inst->t);
  json_object_object_add(obj, "validTime", json_object_new_string(vt));
  pfree(vt);
  json_object_array_add(instants_array, obj);
  return true;
}

/**
 * @brief Build the @p instants array for a TSequence.
 */
static json_object *
tposeseq_to_instants_array(const TSequence *seq, int conformance, int precision)
{
  json_object *arr = json_object_new_array();
  for (int i = 0; i < seq->count; i++)
  {
    if (! tposeinst_append_geopose(arr, TSEQUENCE_INST_N(seq, i),
        conformance, precision))
    {
      json_object_put(arr);
      return NULL;
    }
  }
  return arr;
}

/**
 * @ingroup meos_pose_geopose_accessor
 * @brief Return the OGC GeoPose JSON representation of a temporal pose
 * @details Each instant is emitted as a Basic-class GeoPose object with
 * an added @p validTime member (ISO-8601 timestamptz). The temporal
 * envelope carries the conformance class and interpolation. A
 * @p TSequenceSet emits a top-level @p sequences array; a @p TSequence
 * emits a single @p instants array; a @p TInstant emits an @p instants
 * array with one element.
 * @param[in] temp Temporal pose
 * @param[in] conformance Conformance class (0 = Basic-Quaternion, 1 = Basic-YPR)
 * @param[in] precision Significant digits in JSON numbers; -1 = lossless
 * @return On error return @p NULL
 * @csqlfn #Tpose_as_geopose()
 */
char *
tpose_as_geopose(const Temporal *temp, int conformance, int precision)
{
  VALIDATE_TPOSE(temp, NULL);

  json_object *root = json_object_new_object();
  json_object_object_add(root, "type",
    json_object_new_string("TemporalGeoPose"));
  json_object_object_add(root, "version", json_object_new_string("1.0"));
  json_object_object_add(root, "conformance",
    json_object_new_string(geopose_conformance_name(conformance)));
  json_object_object_add(root, "interpolation",
    json_object_new_string(geopose_interp_name(MEOS_FLAGS_GET_INTERP(temp->flags))));

  switch (temp->subtype)
  {
    case TINSTANT:
    {
      json_object *arr = json_object_new_array();
      if (! tposeinst_append_geopose(arr, (const TInstant *) temp,
          conformance, precision))
      {
        json_object_put(arr);
        json_object_put(root);
        return NULL;
      }
      json_object_object_add(root, "instants", arr);
      break;
    }
    case TSEQUENCE:
    {
      const TSequence *seq = (const TSequence *) temp;
      json_object *arr = tposeseq_to_instants_array(seq, conformance, precision);
      if (arr == NULL) { json_object_put(root); return NULL; }
      json_object_object_add(root, "lower_inc",
        json_object_new_boolean(seq->period.lower_inc));
      json_object_object_add(root, "upper_inc",
        json_object_new_boolean(seq->period.upper_inc));
      json_object_object_add(root, "instants", arr);
      break;
    }
    default: /* TSEQUENCESET */
    {
      const TSequenceSet *ss = (const TSequenceSet *) temp;
      json_object *seqs = json_object_new_array();
      for (int i = 0; i < ss->count; i++)
      {
        const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
        json_object *arr = tposeseq_to_instants_array(seq, conformance,
          precision);
        if (arr == NULL)
        {
          json_object_put(seqs);
          json_object_put(root);
          return NULL;
        }
        json_object *seqobj = json_object_new_object();
        json_object_object_add(seqobj, "lower_inc",
          json_object_new_boolean(seq->period.lower_inc));
        json_object_object_add(seqobj, "upper_inc",
          json_object_new_boolean(seq->period.upper_inc));
        json_object_object_add(seqobj, "instants", arr);
        json_object_array_add(seqs, seqobj);
      }
      json_object_object_add(root, "sequences", seqs);
      break;
    }
  }

  const char *raw = json_object_to_json_string_ext(root, JSON_C_TO_STRING_PLAIN);
  char *out = pstrdup(raw);
  json_object_put(root);
  return out;
}

/**
 * @brief Parse one element of an @p instants array (Basic-class object
 * + @p validTime member) into a TInstant. Returns @p NULL on error.
 */
static TInstant *
tposeinst_from_geopose_object(json_object *obj)
{
  if (! json_object_is_type(obj, json_type_object))
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "TemporalGeoPose 'instants' element must be an object");
    return NULL;
  }
  json_object *jt = geopose_find_member(obj, "validTime");
  if (jt == NULL || ! json_object_is_type(jt, json_type_string))
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "TemporalGeoPose instant missing required 'validTime' string");
    return NULL;
  }
  const char *ts = json_object_get_string(jt);
  TimestampTz t = pg_timestamptz_in(ts, -1);
  Pose *pose = pose_from_geopose_object(obj);
  if (! pose) return NULL;
  TInstant *inst = tinstant_make(PointerGetDatum(pose), T_TPOSE, t);
  pfree(pose);
  return inst;
}

/**
 * @brief Parse a JSON instants array into a heap-allocated array of
 * TInstant pointers. Returns the count or -1 on error; on success the
 * caller owns @p *out_instants and must free it with @p pfree_array.
 */
static int
tpose_parse_instants(json_object *instants_arr, TInstant ***out_instants)
{
  if (! json_object_is_type(instants_arr, json_type_array))
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "TemporalGeoPose 'instants' must be a JSON array");
    return -1;
  }
  int count = (int) json_object_array_length(instants_arr);
  if (count == 0)
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "TemporalGeoPose 'instants' must be non-empty");
    return -1;
  }
  TInstant **insts = palloc(sizeof(TInstant *) * count);
  for (int i = 0; i < count; i++)
  {
    json_object *e = json_object_array_get_idx(instants_arr, i);
    insts[i] = tposeinst_from_geopose_object(e);
    if (insts[i] == NULL)
    {
      for (int j = 0; j < i; j++) pfree(insts[j]);
      pfree(insts);
      return -1;
    }
  }
  *out_instants = insts;
  return count;
}

/**
 * @ingroup meos_pose_geopose_accessor
 * @brief Return a temporal pose from its OGC GeoPose JSON representation
 * @details Parses the @p TemporalGeoPose envelope (see @p tpose_as_geopose
 * for the shape). Each instant's payload is auto-detected as
 * Basic-Quaternion or Basic-YPR by the same rule as the static entry
 * point. The interpolation and bounds-inclusion flags come from the
 * envelope; the resulting tpose has SRID 4326.
 * @param[in] json TemporalGeoPose JSON string
 * @return On error return @p NULL
 * @csqlfn #Tpose_from_geopose()
 */
Temporal *
tpose_from_geopose(const char *json)
{
  if (json == NULL)
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "Null TemporalGeoPose JSON string");
    return NULL;
  }
  json_tokener *tok = json_tokener_new();
  json_object *root = json_tokener_parse_ex(tok, json, -1);
  if (tok->err != json_tokener_success || root == NULL)
  {
    char err[256];
    snprintf(err, sizeof(err), "%s (at offset %d)",
      json_tokener_error_desc(tok->err), tok->char_offset);
    json_tokener_free(tok);
    if (root) json_object_put(root);
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "Invalid TemporalGeoPose JSON: %s", err);
    return NULL;
  }
  json_tokener_free(tok);

  /* Optional envelope sanity: known type. */
  json_object *jtype = geopose_find_member(root, "type");
  if (jtype && json_object_is_type(jtype, json_type_string) &&
      strcmp(json_object_get_string(jtype), "TemporalGeoPose") != 0)
  {
    json_object_put(root);
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "Unknown 'type' in TemporalGeoPose envelope");
    return NULL;
  }

  /* Interpolation. */
  json_object *jinterp = geopose_find_member(root, "interpolation");
  interpType interp = LINEAR;
  if (jinterp && json_object_is_type(jinterp, json_type_string))
    interp = geopose_interp_from_string(json_object_get_string(jinterp));

  /* Sequence set: top-level `sequences` array. */
  json_object *jseqs = geopose_find_member(root, "sequences");
  if (jseqs != NULL)
  {
    if (! json_object_is_type(jseqs, json_type_array))
    {
      json_object_put(root);
      meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
        "TemporalGeoPose 'sequences' must be a JSON array");
      return NULL;
    }
    int nseqs = (int) json_object_array_length(jseqs);
    TSequence **sequences = palloc(sizeof(TSequence *) * nseqs);
    for (int i = 0; i < nseqs; i++)
    {
      json_object *seqobj = json_object_array_get_idx(jseqs, i);
      json_object *jli = geopose_find_member(seqobj, "lower_inc");
      json_object *jui = geopose_find_member(seqobj, "upper_inc");
      bool li = jli ? json_object_get_boolean(jli) : true;
      bool ui = jui ? json_object_get_boolean(jui) : true;
      json_object *jinsts = geopose_find_member(seqobj, "instants");
      if (! jinsts) {
        json_object_put(root);
        for (int j = 0; j < i; j++) pfree(sequences[j]);
        pfree(sequences);
        meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
          "TemporalGeoPose 'sequences' element missing 'instants'");
        return NULL;
      }
      TInstant **insts = NULL;
      int n = tpose_parse_instants(jinsts, &insts);
      if (n < 0) {
        json_object_put(root);
        for (int j = 0; j < i; j++) pfree(sequences[j]);
        pfree(sequences);
        return NULL;
      }
      sequences[i] = tsequence_make(insts, n, li, ui, interp, NORMALIZE);
      pfree_array((void **) insts, n);
    }
    Temporal *result = (Temporal *) tsequenceset_make_free(sequences, nseqs,
      NORMALIZE);
    json_object_put(root);
    return result;
  }

  /* Single instants array: TInstant or TSequence. */
  json_object *jinsts = geopose_find_member(root, "instants");
  if (jinsts == NULL)
  {
    json_object_put(root);
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "TemporalGeoPose envelope missing 'instants' array (or 'sequences')");
    return NULL;
  }
  TInstant **insts = NULL;
  int n = tpose_parse_instants(jinsts, &insts);
  if (n < 0) { json_object_put(root); return NULL; }

  Temporal *result;
  /* A single-instant value with no interpolation (or "None") round-trips
   * back to a TInstant. This matches the asGeoPose emission of TINSTANT
   * which writes "interpolation":"None". */
  if (n == 1 && (jinterp == NULL || interp == INTERP_NONE))
  {
    result = (Temporal *) insts[0];
    pfree(insts);
  }
  else
  {
    json_object *jli = geopose_find_member(root, "lower_inc");
    json_object *jui = geopose_find_member(root, "upper_inc");
    bool li = jli ? json_object_get_boolean(jli) : true;
    bool ui = jui ? json_object_get_boolean(jui) : true;
    result = (Temporal *) tsequence_make(insts, n,
      li, ui, interp, NORMALIZE);
    pfree_array((void **) insts, n);
  }
  json_object_put(root);
  return result;
}

/*****************************************************************************/
