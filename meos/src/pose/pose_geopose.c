/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
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
 * @brief OGC GeoPose JSON I/O — the classes a single pose is written as.
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
 * The Advanced class carries the same pose with the frame named explicitly:
 *
 *   {
 *     "frameSpecification": {
 *       "authority": "/geopose/1.0",
 *       "id": "LTP-ENU",
 *       "parameters": "longitude=8&latitude=47&height=1500&crs=EPSG:4979"
 *     },
 *     "quaternion": { "x": 0.0, "y": 0.0, "z": 0.7071, "w": 0.7071 }
 *   }
 *
 * The class has no position member, so the pose sits at the tangent point of
 * the frame it names, which makes the two documents above describe the same
 * pose and read back equal.
 *
 * On input the parser auto-detects the conformance class from the keys
 * present (`frameSpecification`, then `quaternion` vs `angles`). On output
 * the caller picks the class via `GeoPoseClass`; the default chosen by the
 * SQL surface is Basic-Quaternion since it is lossless for our internal
 * representation.
 *
 * 2D poses (data = [x, y, theta]) are represented in JSON with `h: 0`,
 * `pitch: 0`, `roll: 0`, and yaw = theta. `theta` is stored in radians
 * internally; the JSON form uses degrees, per the standard.
 *
 * A temporal pose is written as an OGC Composite Sequence Series, either
 * Regular or Irregular. Those classes encode each pose as an inner
 * `FrameSpecification` holding a translation and a rotation relative to a
 * single outer LTP-ENU frame, rather than as a Basic pose object, so the
 * series encoding carries its own geographic-to-topocentric conversion.
 * The MobilityDB `TemporalGeoPose` envelope, an array of Basic-class
 * objects each carrying a `validTime`, remains available for reading and
 * writing but is not an OGC conformance class.
 *
 * A Stream is written by the two entry points of its own, one for the header
 * that opens it and one per element. The Chain and Graph classes are not
 * implemented.
 */

/* C */
#include <math.h>
/* json-c */
#include <json-c/json.h>
/* PostgreSQL */
#include <postgres.h>
#include <varatt.h>
/* MEOS */
#include <meos.h>
#include <meos_pose.h>
#include <meos_internal.h>
#include <pgtypes.h>                  /* pg_timestamptz_in / pg_timestamptz_out */
#include "temporal/temporal.h"      /* ensure_not_null */
#include "temporal/type_util.h"     /* pfree_array */
#include "pose/pose.h"
#include "pose/pose_geopose.h"
#if POSE
  #include "pose/posechain.h"
#endif

/*****************************************************************************
 * Helpers
 *****************************************************************************/

/* Every GeoPose class fixes the outer frame to WGS-84 geographic. Two EPSG
 * codes name that frame: 4326, which is two-dimensional, and 4979, which is
 * three-dimensional. Their proj4 definitions are identical
 * (`+proj=longlat +datum=WGS84 +no_defs`), the codes differing only in a
 * declared dimensionality that proj4 does not express, so both are accepted
 * and neither moves a coordinate. SRID 0 is accepted and treated as
 * geographic.
 *
 * A GeoPose position is `{lat, lon, h}`, so 4979 is the code that describes
 * it, and it is the one the composite outer frame declares. Values keep the
 * SRID 4326 that the rest of the pose surface uses. */
#define GEOPOSE_SRID_WGS84_2D 4326
#define GEOPOSE_SRID_WGS84_3D 4979
#define GEOPOSE_GEOGRAPHIC_SRID GEOPOSE_SRID_WGS84_2D

/* The authority and identifiers naming the two frames, as used by the
 * encoding examples of OGC 21-056r11 Clause 9.2. The Advanced class names its
 * outer frame and the composites name both, so these are shared. */
#define GEOPOSE_AUTHORITY       "/geopose/1.0"
#define GEOPOSE_ID_OUTER_FRAME  "LTP-ENU"
#define GEOPOSE_ID_INNER_FRAME  "RotateTranslate"
/* The Chain class names the same two frames differently: its normative
 * instance writes `/Extrinsic/LTP-ENU` and `/Intrinsic/Translate-Rotate`
 * where the Advanced, Series and Stream instances write the bare pair above,
 * under the one authority. A document is written with the pair its own class
 * uses, and a Chain is read accepting either. */
#define GEOPOSE_ID_CHAIN_OUTER_FRAME  "/Extrinsic/LTP-ENU"
#define GEOPOSE_ID_CHAIN_INNER_FRAME  "/Intrinsic/Translate-Rotate"

/*****************************************************************************
 * The frame registry
 *****************************************************************************/

/**
 * @brief Every frame an encoder in this file names, and the two systems the
 * Basic classes rest on
 * @details The four rows under #GEOPOSE_AUTHORITY take their identifiers from
 * the very macros the encoders write, so a frame this file learns to emit
 * cannot be absent here, and a host that materialises the registry as a table
 * states what the documents state.
 */
static const GeoPoseFrame GEOPOSE_FRAME_REGISTRY[] =
{
  { 1, "EPSG", "4326", "WGS-84 geographic (lat/lon/h)", 4326, true,
    "OGC GeoPose Basic-class default outer frame. Position parsed as "
    "{lat, lon, h} in degrees / metres." },
  { 2, "EPSG", "4978", "WGS-84 ECEF (Earth-Centred Earth-Fixed)", 4978, false,
    "Cartesian X/Y/Z geocentric. Used as an intermediate by frame transforms; "
    "rotation between this frame and WGS-84 geographic at point P is given by "
    "the East-North-Up basis at P." },
  { 3, "OGC", "LTP", "Local Tangent Plane (East-North-Up)", 0, false,
    "Parameterised at runtime by an anchor (lat, lon, h). The outer-frame "
    "conversion to ECEF is the standard ENU rotation matrix at the anchor." },
  { 4, "OGC", "BODY", "Right-handed body axes (default inner frame)", 0, false,
    "Conventional inner frame: X forward, Y left, Z up. The pose's quaternion "
    "takes vectors from this body frame to the outer frame." },
  { 5, GEOPOSE_AUTHORITY, GEOPOSE_ID_OUTER_FRAME,
    "GeoPose outer frame of a Composite Sequence Series", 0, false,
    "Authority and id emitted as the outerFrame of a Series. Parameterised by "
    "the tangent point of the first pose as "
    "'longitude=<degrees>&latitude=<degrees>&height=<metres>'." },
  { 6, GEOPOSE_AUTHORITY, GEOPOSE_ID_INNER_FRAME,
    "GeoPose inner frame of a Composite Sequence Series", 0, false,
    "Authority and id emitted for each inner frame of a Series. Parameterised "
    "as 'translation=[e, n, u]&rotation=[w, x, y, z]', the translation in "
    "metres in the outer frame and the rotation taking body axes to the outer "
    "frame." },
  { 7, GEOPOSE_AUTHORITY, GEOPOSE_ID_CHAIN_OUTER_FRAME,
    "GeoPose outer frame of a Chain", 0, false,
    "The frame a Chain document names where a Series names 'LTP-ENU'. Same "
    "frame, same authority, the name its own class uses; a Chain is read "
    "accepting either." },
  { 8, GEOPOSE_AUTHORITY, GEOPOSE_ID_CHAIN_INNER_FRAME,
    "GeoPose inner frame of a Chain", 0, false,
    "The frame a Chain document names where a Series names 'RotateTranslate'. "
    "Same frame, same authority, the name its own class uses; a Chain is read "
    "accepting either." }
};

/**
 * @ingroup meos_pose_base_geopose
 * @brief Return every frame of the OGC GeoPose registry
 * @param[out] count Number of frames returned
 * @csqlfn #Geopose_frames()
 */
const GeoPoseFrame *
geopose_frames(int *count)
{
  VALIDATE_NOT_NULL(count, NULL);
  *count = (int) (sizeof(GEOPOSE_FRAME_REGISTRY) /
    sizeof(GEOPOSE_FRAME_REGISTRY[0]));
  return GEOPOSE_FRAME_REGISTRY;
}

/**
 * @ingroup meos_pose_base_geopose
 * @brief Return the frame the registry states under an identifier, or `NULL`
 * where it states none
 * @param[in] frame_id Identifier of the frame
 */
const GeoPoseFrame *
geopose_frame(int32_t frame_id)
{
  int count = (int) (sizeof(GEOPOSE_FRAME_REGISTRY) /
    sizeof(GEOPOSE_FRAME_REGISTRY[0]));
  for (int i = 0; i < count; i++)
    if (GEOPOSE_FRAME_REGISTRY[i].frame_id == frame_id)
      return &GEOPOSE_FRAME_REGISTRY[i];
  return NULL;
}

/**
 * @brief Return true if @p srid names the WGS-84 geographic frame the
 * GeoPose classes require, or is unknown.
 */
static bool
geopose_srid_is_geographic(int32_t srid)
{
  return srid == 0 || srid == GEOPOSE_SRID_WGS84_2D ||
    srid == GEOPOSE_SRID_WGS84_3D;
}

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

/* Serialization flags: compact, and with '/' left unescaped so that the
 * emitted authority strings read as the standard writes them. */
#define GEOPOSE_JSON_FLAGS \
  (JSON_C_TO_STRING_PLAIN | JSON_C_TO_STRING_NOSLASHESCAPE)

#define GEOPOSE_DEG2RAD(d) ((d) * (M_PI / 180.0))
#define GEOPOSE_RAD2DEG(r) ((r) * (180.0 / M_PI))

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

/**
 * @brief Return the value of @p key in a `key=value&key=value` parameter
 * string, or @p NULL if the key is absent.
 */
static const char *
geopose_param_find(const char *params, const char *key)
{
  size_t klen = strlen(key);
  const char *p = params;
  while (p != NULL && *p != '\0')
  {
    if (pg_strncasecmp(p, key, klen) == 0 && p[klen] == '=')
      return p + klen + 1;
    p = strchr(p, '&');
    if (p != NULL)
      p++;
  }
  return NULL;
}

/**
 * @brief Read a bracketed list of @p n numbers, as emitted for the
 * `translation` and `rotation` parameters.
 */
static bool
geopose_param_list(const char *str, int n, double *out)
{
  char *end;
  if (str == NULL)
    return false;
  while (*str == ' ')
    str++;
  if (*str != '[')
    return false;
  str++;
  for (int i = 0; i < n; i++)
  {
    out[i] = strtod(str, &end);
    if (end == str)
      return false;
    str = end;
    while (*str == ' ')
      str++;
    if (i < n - 1)
    {
      if (*str != ',')
        return false;
      str++;
    }
  }
  return (*str == ']');
}

/**
 * @brief Read a single number parameter, as emitted for the outer frame's
 * `longitude`, `latitude` and `height`.
 */
static bool
geopose_param_number(const char *str, double *out)
{
  char *end;
  if (str == NULL)
    return false;
  *out = strtod(str, &end);
  return (end != str);
}

/**
 * @brief Extract the position and orientation of a pose in the form every
 * GeoPose encoding needs: longitude, latitude and height in degrees and
 * metres, and a unit quaternion in Hamilton convention.
 * @details Every class the standard defines places its pose in a topocentric
 * frame on the surface of the Earth, so this is where the frame of the pose
 * is checked. A pose with SRID 0 is treated as geographic.
 *
 * That frame is geographic, which a planar pose does not have: its
 * coordinates measure a plane rather than the ellipsoid, so writing them as a
 * longitude and a latitude would assert something the value does not say. A
 * planar pose is therefore reported here rather than encoded.
 * @return On error return false
 */
static bool
geopose_pose_components(const Pose *pose, double *lon, double *lat, double *h,
  double *W, double *X, double *Y, double *Z)
{
  if (! MEOS_FLAGS_GET_GEODETIC(pose->flags))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "GeoPose JSON requires a geodetic pose, got a planar one");
    return false;
  }
  int32_t srid = pose_srid(pose);
  if (! geopose_srid_is_geographic(srid))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "GeoPose JSON requires a WGS-84 geographic SRID (4326 or 4979), "
      "got %d", srid);
    return false;
  }
  bool has_z = MEOS_FLAGS_GET_Z(pose->flags);
  *lon = pose->data[0];
  *lat = pose->data[1];
  *h = has_z ? pose->data[2] : 0.0;
  if (has_z)
  {
    *W = pose->data[3]; *X = pose->data[4];
    *Y = pose->data[5]; *Z = pose->data[6];
  }
  else
    /* 2D pose: pure yaw rotation about the local vertical. */
    pose_ypr_to_quaternion(pose->data[2], 0.0, 0.0, W, X, Y, Z);
  return true;
}

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
 * @brief Read the `quaternion` member shared by the Basic-Quaternion and
 * Advanced classes.
 * @return On error return false
 */
static bool
geopose_quaternion_from_json(json_object *jq, double *W, double *X, double *Y,
  double *Z)
{
  if (! geopose_get_number(jq, "w", W) || ! geopose_get_number(jq, "x", X) ||
      ! geopose_get_number(jq, "y", Y) || ! geopose_get_number(jq, "z", Z))
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "GeoPose 'quaternion' must carry numeric 'w','x','y','z' members");
    return false;
  }
  return true;
}

/**
 * @brief Return whether a `crs` frame parameter names a WGS-84 geographic
 * CRS, in the `EPSG:<code>` form the frames of this module carry and the
 * form the Moving Features encoding names a CRS by.
 */
static bool
geopose_crs_is_geographic(const char *crs)
{
  if (pg_strncasecmp(crs, "EPSG:", 5) != 0)
    return false;
  char *end;
  long code = strtol(crs + 5, &end, 10);
  return (end != crs + 5) && geopose_srid_is_geographic((int32_t) code);
}

/**
 * @brief Build a pose from a parsed Advanced-class GeoPose JSON object.
 * @details An Advanced document names its outer frame explicitly and has no
 * `position` member, so the pose is placed by the frame: it sits at the
 * tangent point the frame's `parameters` give. Requirement 9 leaves those
 * parameters free-form and the standard registers no authority or id, so what
 * is read here is what this module writes, which is also what the encoding
 * examples of the standard use.
 *
 * A frame this module cannot place a pose in is reported rather than guessed
 * at, and so is a missing coordinate: a pose whose position defaulted to zero
 * would be accepted here and refused by every operation that followed.
 * @return On error return @p NULL
 */
static Pose *
pose_from_geopose_advanced(json_object *frame, json_object *root)
{
  if (! json_object_is_type(frame, json_type_object))
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "GeoPose 'frameSpecification' must be a FrameSpecification object");
    return NULL;
  }
  json_object *jauth = geopose_find_member(frame, "authority");
  json_object *jid = geopose_find_member(frame, "id");
  json_object *jpar = geopose_find_member(frame, "parameters");
  if (jauth == NULL || ! json_object_is_type(jauth, json_type_string) ||
      jid == NULL || ! json_object_is_type(jid, json_type_string) ||
      jpar == NULL || ! json_object_is_type(jpar, json_type_string))
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "GeoPose 'frameSpecification' requires string 'authority', 'id' and "
      "'parameters' members");
    return NULL;
  }

  const char *auth = json_object_get_string(jauth);
  const char *id = json_object_get_string(jid);
  if (strcmp(auth, GEOPOSE_AUTHORITY) != 0 ||
      pg_strcasecmp(id, GEOPOSE_ID_OUTER_FRAME) != 0)
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "Unsupported GeoPose outer frame '%s'/'%s'; the frame read is "
      "'%s'/'%s', which carries the position of the pose in its parameters",
      auth, id, GEOPOSE_AUTHORITY, GEOPOSE_ID_OUTER_FRAME);
    return NULL;
  }

  const char *params = json_object_get_string(jpar);
  double lon, lat, h;
  if (! geopose_param_number(geopose_param_find(params, "longitude"), &lon) ||
      ! geopose_param_number(geopose_param_find(params, "latitude"), &lat) ||
      ! geopose_param_number(geopose_param_find(params, "height"), &h))
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "GeoPose 'frameSpecification' parameters must carry numeric "
      "'longitude', 'latitude' and 'height'");
    return NULL;
  }

  /* The frame may name the CRS its tangent point is given in. A projected one
   * would place the pose off the ellipsoid the topocentric conversion is
   * against, so only a geographic CRS is read. */
  const char *crs = geopose_param_find(params, "crs");
  if (crs != NULL && ! geopose_crs_is_geographic(crs))
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "GeoPose 'frameSpecification' parameter 'crs' must name a WGS-84 "
      "geographic CRS as EPSG:%d or EPSG:%d", GEOPOSE_SRID_WGS84_2D,
      GEOPOSE_SRID_WGS84_3D);
    return NULL;
  }

  json_object *jq = geopose_find_member(root, "quaternion");
  if (jq == NULL || ! json_object_is_type(jq, json_type_object))
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "GeoPose Advanced JSON missing required 'quaternion' object");
    return NULL;
  }
  double W, X, Y, Z;
  if (! geopose_quaternion_from_json(jq, &W, &X, &Y, &Z))
    return NULL;
  return pose_make_3d(lon, lat, h, W, X, Y, Z, true,
    GEOPOSE_GEOGRAPHIC_SRID);
}

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
  /* An Advanced document names its outer frame explicitly and carries no
   * `position`, the placement travelling in the frame instead. */
  json_object *jframe = geopose_find_member(root, "frameSpecification");
  if (jframe != NULL)
    return pose_from_geopose_advanced(jframe, root);

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
    if (! geopose_quaternion_from_json(jq, &W, &X, &Y, &Z))
      return NULL;
    /* Note: any reasonable client emits a unit quaternion. We keep the
     * input verbatim — the canonicalization/normalization pass is a
     * separate phase. */
    result = pose_make_3d(lon, lat, h, W, X, Y, Z, true,
      GEOPOSE_GEOGRAPHIC_SRID);
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
      result = pose_make_2d(lon, lat, theta_rad, true,
        GEOPOSE_GEOGRAPHIC_SRID);
    }
    else
    {
      double W, X, Y, Z;
      pose_ypr_to_quaternion(GEOPOSE_DEG2RAD(yaw_deg),
        GEOPOSE_DEG2RAD(pitch_deg), GEOPOSE_DEG2RAD(roll_deg),
        &W, &X, &Y, &Z);
      result = pose_make_3d(lon, lat, h, W, X, Y, Z, true,
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
 * @param[in] json GeoPose JSON string (Basic-YPR, Basic-Quaternion or
 * Advanced)
 * @return On error return @p NULL
 * @details Auto-detects the conformance class from the JSON keys present:
 *
 *   - `frameSpecification`: Advanced (returns a 3D pose placed at the
 *     tangent point of the frame, which is where a class with no position
 *     member of its own carries it).
 *   - `quaternion`: Basic-Quaternion (returns a 3D pose).
 *   - `angles`:     Basic-YPR (returns a 3D pose; or 2D if pitch=roll=0
 *     and h=0, since that is the canonical 2D representation).
 *
 * A Basic position is parsed as `{lat, lon, h}` (degrees, degrees, metres). The
 * resulting pose has SRID 4326 (WGS-84). Projected SRIDs are not
 * supported by this entry point — use the WKT/WKB I/O for those.
 * @csqlfn #Pose_from_geopose()
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

/* The Advanced class places its pose in an explicit topocentric frame, which
 * the geodesy section below builds, so its document is assembled there. */
static json_object *pose_to_geopose_advanced(const Pose *pose, int precision);

/**
 * @brief Build the GeoPose JSON object for a single pose.
 * @details Used internally by @p pose_as_geopose (single-pose entry
 * point) and by the temporal-GeoPose entry points which embed the
 * per-instant object into an envelope's @p instants array. The caller
 * owns the returned object and is responsible for releasing it.
 * @return On error return @p NULL.
 */
static json_object *
pose_to_geopose_object(const Pose *pose, int conformance, int precision)
{
  if (conformance == GEOPOSE_ADVANCED)
    return pose_to_geopose_advanced(pose, precision);

  if (conformance != GEOPOSE_BASIC_QUATERNION &&
      conformance != GEOPOSE_BASIC_YPR)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Unknown GeoPose conformance class %d "
      "(0 = Basic-Quaternion, 1 = Basic-YPR, 2 = Advanced)", conformance);
    return NULL;
  }
  /* Both classes start from a quaternion, and both place the position in
   * the same geographic outer frame. */
  double lon, lat, h, W, X, Y, Z;
  if (! geopose_pose_components(pose, &lon, &lat, &h, &W, &X, &Y, &Z))
    return NULL;

  json_object *root = json_object_new_object();
  json_object *jpos = json_object_new_object();
  json_object_object_add(jpos, "lat", geopose_new_double(lat, precision));
  json_object_object_add(jpos, "lon", geopose_new_double(lon, precision));
  /* `h` is required, so a 2D pose emits `h: 0`. GeoPose offers no way to
   * say that a height is unknown, so zero is the convention for a
   * height-free trajectory; it asserts more than the data holds. */
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
    pose_quaternion_to_ypr(W, X, Y, Z, &yaw_rad, &pitch_rad, &roll_rad);
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
 * @param[in] conformance Conformance class to emit
 * (0 = Basic-Quaternion, 1 = Basic-YPR, 2 = Advanced)
 * @param[in] precision Decimal places to keep in the JSON numbers; pass
 * a negative value to use json-c's default
 * @return On error return @p NULL
 * @details Every conformance class mandates a geographic outer
 * frame. Poses with SRID 0 are treated as geographic; poses with
 * non-zero non-4326 SRIDs are rejected (use WKT/WKB instead).
 *
 * The Basic classes carry the position in a `position` member and leave the
 * frame implicit; the Advanced class names the frame explicitly and carries
 * the position in it, the class having no position member of its own. The
 * frame it names is the one at the pose, so the two documents describe the
 * same pose and read back equal.
 * @csqlfn #Pose_as_geopose()
 */
char *
pose_as_geopose(const Pose *pose, int conformance, int precision)
{
  VALIDATE_NOT_NULL(pose, NULL);
  json_object *root = pose_to_geopose_object(pose, conformance, precision);
  if (root == NULL)
    return NULL;

  int flags = GEOPOSE_JSON_FLAGS;
  const char *raw = json_object_to_json_string_ext(root, flags);
  char *out = pstrdup(raw);
  json_object_put(root);
  return out;
}

/*****************************************************************************
 * OGC GeoPose Composite Sequence classes — geodesy
 *
 * The Composite Sequence classes do not embed Basic pose objects. Every
 * inner frame is a FrameSpecification whose `parameters` string carries a
 * translation and a rotation relative to a single outer frame, which the
 * standard's examples realize as an LTP-ENU frame anchored at an explicit
 * tangent point. Emitting a conformant series therefore requires the
 * geographic-to-topocentric conversion implemented here.
 *****************************************************************************/

/* WGS-84 defining parameters (EPSG::7030). The GeoPose Basic classes fix
 * the outer frame to an implicit WGS-84 CRS (Requirements 12 and 14), so
 * the topocentric conversion is against this ellipsoid. */
#define GEOPOSE_WGS84_A   6378137.0
#define GEOPOSE_WGS84_F   (1.0 / 298.257223563)
#define GEOPOSE_WGS84_E2  (GEOPOSE_WGS84_F * (2.0 - GEOPOSE_WGS84_F))

/**
 * @brief Convert WGS-84 geographic coordinates to Earth-Centred
 * Earth-Fixed Cartesian coordinates.
 */
static void
geopose_geodetic_to_ecef(double lat_rad, double lon_rad, double h,
  double *X, double *Y, double *Z)
{
  double s = sin(lat_rad), c = cos(lat_rad);
  double N = GEOPOSE_WGS84_A / sqrt(1.0 - GEOPOSE_WGS84_E2 * s * s);
  *X = (N + h) * c * cos(lon_rad);
  *Y = (N + h) * c * sin(lon_rad);
  *Z = (N * (1.0 - GEOPOSE_WGS84_E2) + h) * s;
}

/**
 * @brief Recover the ellipsoidal height once the latitude is known.
 * @details Uses whichever of the two equivalent expressions is better
 * conditioned: @p p / cos φ degenerates at the poles, @p Z / sin φ at the
 * equator. The switch is at |φ| = 45°, where both are equally conditioned.
 */
static double
geopose_ecef_height(double p, double Z, double lat_rad, double N)
{
  double s = sin(lat_rad);
  if (fabs(s) > M_SQRT1_2)
    return Z / s - N * (1.0 - GEOPOSE_WGS84_E2);
  return p / cos(lat_rad) - N;
}

/**
 * @brief Convert Earth-Centred Earth-Fixed Cartesian coordinates to WGS-84
 * geographic coordinates.
 * @details Fixed-point iteration on (φ, h) seeded with the spherical
 * approximation. The iteration count is fixed rather than
 * convergence-tested so that every language binding computes exactly the
 * same result from the same input, which this project requires of any
 * value that reaches a serialization.
 */
static void
geopose_ecef_to_geodetic(double X, double Y, double Z,
  double *lat_rad, double *lon_rad, double *h)
{
  double p = sqrt(X * X + Y * Y);
  if (p == 0.0)
  {
    /* On the polar axis the longitude is undefined; report it as zero. */
    double b = GEOPOSE_WGS84_A * sqrt(1.0 - GEOPOSE_WGS84_E2);
    *lon_rad = 0.0;
    *lat_rad = (Z >= 0.0) ? M_PI_2 : -M_PI_2;
    *h = fabs(Z) - b;
    return;
  }
  *lon_rad = atan2(Y, X);
  double lat = atan2(Z, p * (1.0 - GEOPOSE_WGS84_E2));
  double N = GEOPOSE_WGS84_A;
  for (int i = 0; i < 8; i++)
  {
    double s = sin(lat);
    N = GEOPOSE_WGS84_A / sqrt(1.0 - GEOPOSE_WGS84_E2 * s * s);
    double height = geopose_ecef_height(p, Z, lat, N);
    lat = atan2(Z, p * (1.0 - GEOPOSE_WGS84_E2 * N / (N + height)));
  }
  double s = sin(lat);
  N = GEOPOSE_WGS84_A / sqrt(1.0 - GEOPOSE_WGS84_E2 * s * s);
  *lat_rad = lat;
  *h = geopose_ecef_height(p, Z, lat, N);
}

/**
 * @brief The outer frame of a Composite Sequence: an LTP-ENU frame
 * anchored at a tangent point, cached with everything the per-instant
 * conversions need.
 */
typedef struct
{
  double lat_rad;             /**< Tangent point latitude */
  double lon_rad;             /**< Tangent point longitude */
  double h;                   /**< Tangent point ellipsoidal height */
  double X, Y, Z;             /**< Tangent point in ECEF */
  double qw, qx, qy, qz;      /**< Rotation ECEF -> this frame's ENU basis */
} GeoPoseAnchor;

/**
 * @brief Initialize the outer-frame anchor at a geographic tangent point.
 * @details What the anchor holds is the rotation that takes ECEF components
 * to ENU components at the tangent point, which is the conjugate of the one
 * @p pose_enu_to_ecef_quaternion builds, both being unit.
 */
static void
geopose_anchor_set(GeoPoseAnchor *anchor, double lat_rad, double lon_rad,
  double h)
{
  anchor->lat_rad = lat_rad;
  anchor->lon_rad = lon_rad;
  anchor->h = h;
  geopose_geodetic_to_ecef(lat_rad, lon_rad, h, &anchor->X, &anchor->Y,
    &anchor->Z);
  pose_enu_to_ecef_quaternion(lat_rad, lon_rad, &anchor->qw, &anchor->qx,
    &anchor->qy, &anchor->qz);
  anchor->qx = -anchor->qx;
  anchor->qy = -anchor->qy;
  anchor->qz = -anchor->qz;
}

/**
 * @brief Return the translation of a geographic point in the anchor's
 * LTP-ENU frame, in metres.
 */
static void
geopose_anchor_translation(const GeoPoseAnchor *anchor, double lat_rad,
  double lon_rad, double h, double *e, double *n, double *u)
{
  double X, Y, Z;
  geopose_geodetic_to_ecef(lat_rad, lon_rad, h, &X, &Y, &Z);
  double dx = X - anchor->X, dy = Y - anchor->Y, dz = Z - anchor->Z;
  double sl = sin(anchor->lon_rad), cl = cos(anchor->lon_rad);
  double sp = sin(anchor->lat_rad), cp = cos(anchor->lat_rad);
  *e = -sl * dx + cl * dy;
  *n = -sp * cl * dx - sp * sl * dy + cp * dz;
  *u =  cp * cl * dx + cp * sl * dy + sp * dz;
}

/**
 * @brief Return the geographic coordinates of a translation expressed in
 * the anchor's LTP-ENU frame. Inverse of @p geopose_anchor_translation.
 */
static void
geopose_anchor_position(const GeoPoseAnchor *anchor, double e, double n,
  double u, double *lat_rad, double *lon_rad, double *h)
{
  double sl = sin(anchor->lon_rad), cl = cos(anchor->lon_rad);
  double sp = sin(anchor->lat_rad), cp = cos(anchor->lat_rad);
  double X = anchor->X - sl * e - sp * cl * n + cp * cl * u;
  double Y = anchor->Y + cl * e - sp * sl * n + cp * sl * u;
  double Z = anchor->Z + cp * n + sp * u;
  geopose_ecef_to_geodetic(X, Y, Z, lat_rad, lon_rad, h);
}

/**
 * @brief Return the rotation that re-expresses an orientation given in the
 * local ENU basis at (@p lat_rad, @p lon_rad) in the anchor's ENU basis.
 * @details A Basic-class pose orients its body frame against the ENU basis
 * at its *own* position, while a Composite Sequence inner frame orients it
 * against the *outer* frame's basis. The two differ by the convergence of
 * the two tangent planes, which is this rotation. Ignoring it would place
 * every pose but the first at a subtly wrong attitude.
 */
static void
geopose_anchor_rotation(const GeoPoseAnchor *anchor, double lat_rad,
  double lon_rad, double *W, double *X, double *Y, double *Z)
{
  double lw, lx, ly, lz;
  pose_enu_to_ecef_quaternion(lat_rad, lon_rad, &lw, &lx, &ly, &lz);
  /* q(R_anchorENU<-ECEF) * q(R_ECEF<-localENU), which are what the anchor
   * holds and what the call above returns. */
  pose_quaternion_mul(anchor->qw, anchor->qx, anchor->qy, anchor->qz,
    lw, lx, ly, lz, W, X, Y, Z);
}

/*****************************************************************************
 * OGC GeoPose Composite Sequence classes — JSON encoding
 *****************************************************************************/

/* The transition model identifiers, from the same encoding examples as the
 * frame identifiers above. */
#define GEOPOSE_ID_TM_NONE      "none"
#define GEOPOSE_ID_TM_INTERP    "interpolate"
/* Requirement 9 places the content of `parameters` outside the scope of
 * GeoPose, so the transition model carries the interpolation there, spelled
 * with the same words the MF-JSON encoding uses in
 * `meos/src/temporal/type_out.c`. One vocabulary serves both of this
 * implementation's OGC encodings, and the round trip stays lossless where
 * the two attested enumeration literals alone would not distinguish step
 * from discrete interpolation. */
#define GEOPOSE_TM_PARAM_KEY    "interpolation"

/* A GeoPose_Instant is Unix time in milliseconds: Requirement 10 states it
 * "shall express Unix Time in seconds multiplied by 1,000, with the unit of
 * measure in milliseconds", and Annex C.2 that time values are encoded as
 * integers needing 64 bits. The offset comes from the one epoch constant the
 * codebase keeps, scaled from its seconds to those milliseconds. */
#define GEOPOSE_UNIX_EPOCH_OFFSET_MS \
  (((int64) DELTA_UNIX_POSTGRES_EPOCH) * 1000)

/**
 * @brief Convert a timestamp to a GeoPose_Instant (Unix milliseconds).
 * @details Rounds towards negative infinity so that the sub-millisecond
 * remainder of a pre-2000 timestamp is dropped the same way as that of a
 * post-2000 one, keeping the mapping monotonic.
 */
static int64
geopose_instant_out(TimestampTz t)
{
  int64 us = (int64) t;
  int64 ms = us / 1000;
  if (us % 1000 != 0 && us < 0)
    ms -= 1;
  return ms + GEOPOSE_UNIX_EPOCH_OFFSET_MS;
}

/**
 * @brief Convert a GeoPose_Instant (Unix milliseconds) to a timestamp.
 */
static TimestampTz
geopose_instant_in(int64 instant)
{
  return (TimestampTz) ((instant - GEOPOSE_UNIX_EPOCH_OFFSET_MS) * 1000);
}

/**
 * @brief Format a double into @p buf with the module's precision
 * convention: @p precision significant digits, or lossless if negative.
 */
static void
geopose_str_double(char *buf, size_t size, double v, int precision)
{
  snprintf(buf, size, "%.*g", (precision < 0) ? 17 : precision, v);
}

/**
 * @brief Build a FrameSpecification object. All three members are required
 * by the schema, so @p parameters is emitted even when empty.
 */
static json_object *
geopose_frame_spec(const char *id, const char *parameters)
{
  json_object *res = json_object_new_object();
  json_object_object_add(res, "authority",
    json_object_new_string(GEOPOSE_AUTHORITY));
  json_object_object_add(res, "id", json_object_new_string(id));
  json_object_object_add(res, "parameters",
    json_object_new_string(parameters));
  return res;
}

/**
 * @brief Build the TransitionModel for a MobilityDB interpolation.
 * @details Requirement 35 asks for an instance of the TransitionModel
 * enumeration, whose literals `interpolate` and `none` are the ones the
 * encoding examples of the standard attest. Linear interpolation is
 * `interpolate`; step and discrete interpolation both estimate nothing
 * between poses and are `none`. The `parameters` string then names the
 * interpolation exactly, which is what tells those two apart on the way
 * back in.
 */
static json_object *
geopose_transition_model(interpType interp)
{
  char params[64];
  snprintf(params, sizeof(params), "%s=%s", GEOPOSE_TM_PARAM_KEY,
    geopose_interp_name(interp));
  json_object *res = json_object_new_object();
  json_object_object_add(res, "authority",
    json_object_new_string(GEOPOSE_AUTHORITY));
  json_object_object_add(res, "id", json_object_new_string(
    (interp == LINEAR) ? GEOPOSE_ID_TM_INTERP : GEOPOSE_ID_TM_NONE));
  json_object_object_add(res, "parameters", json_object_new_string(params));
  return res;
}

/**
 * @brief Recover a MobilityDB interpolation from a TransitionModel object.
 * @details The `parameters` string is authoritative when it names an
 * interpolation, since the two enumeration literals cannot distinguish step
 * from discrete interpolation on their own. A document from another
 * implementation carries no such name, and then `interpolate` reads as
 * linear and anything else as discrete.
 */
static interpType
geopose_transition_model_interp(json_object *tm)
{
  json_object *jpar = geopose_find_member(tm, "parameters");
  if (jpar != NULL && json_object_is_type(jpar, json_type_string))
  {
    const char *val = geopose_param_find(json_object_get_string(jpar),
      GEOPOSE_TM_PARAM_KEY);
    if (val != NULL)
    {
      interpType interp = geopose_interp_from_string(val);
      if (interp != INTERP_NONE)
        return interp;
    }
  }
  json_object *jid = geopose_find_member(tm, "id");
  if (jid == NULL || ! json_object_is_type(jid, json_type_string))
    return LINEAR;
  return (pg_strcasecmp(json_object_get_string(jid),
    GEOPOSE_ID_TM_INTERP) == 0) ? LINEAR : DISCRETE;
}

/**
 * @brief Build the inner FrameSpecification of a pose relative to the
 * outer frame, in the `translation=[…]&rotation=[…]` form used by the
 * encoding examples of the standard.
 * @details The rotation list is ordered w, x, y, z, the order in which
 * Requirement 15 lists the components of a GeoPose quaternion.
 * @return On error return @p NULL
 */
static json_object *
geopose_inner_frame(const char *id, const GeoPoseAnchor *anchor,
  const Pose *pose,
  int precision)
{
  double lon, lat, h, W, X, Y, Z;
  if (! geopose_pose_components(pose, &lon, &lat, &h, &W, &X, &Y, &Z))
    return NULL;
  double lat_rad = GEOPOSE_DEG2RAD(lat), lon_rad = GEOPOSE_DEG2RAD(lon);

  double e, n, u;
  geopose_anchor_translation(anchor, lat_rad, lon_rad, h, &e, &n, &u);

  double rw, rx, ry, rz, qw, qx, qy, qz;
  geopose_anchor_rotation(anchor, lat_rad, lon_rad, &rw, &rx, &ry, &rz);
  pose_quaternion_mul(rw, rx, ry, rz, W, X, Y, Z, &qw, &qx, &qy, &qz);

  char be[64], bn[64], bu[64], bw[64], bx[64], by[64], bz[64];
  geopose_str_double(be, sizeof(be), e, precision);
  geopose_str_double(bn, sizeof(bn), n, precision);
  geopose_str_double(bu, sizeof(bu), u, precision);
  geopose_str_double(bw, sizeof(bw), qw, precision);
  geopose_str_double(bx, sizeof(bx), qx, precision);
  geopose_str_double(by, sizeof(by), qy, precision);
  geopose_str_double(bz, sizeof(bz), qz, precision);

  char params[512];
  snprintf(params, sizeof(params),
    "translation=[%s, %s, %s]&rotation=[%s, %s, %s, %s]",
    be, bn, bu, bw, bx, by, bz);
  return geopose_frame_spec(id, params);
}

/**
 * @brief Build the outer FrameSpecification: the LTP-ENU frame at the
 * anchor's tangent point.
 */
static json_object *
geopose_outer_frame(const char *id, const GeoPoseAnchor *anchor,
  int precision)
{
  char blon[64], blat[64], bh[64];
  geopose_str_double(blon, sizeof(blon), GEOPOSE_RAD2DEG(anchor->lon_rad),
    precision);
  geopose_str_double(blat, sizeof(blat), GEOPOSE_RAD2DEG(anchor->lat_rad),
    precision);
  geopose_str_double(bh, sizeof(bh), anchor->h, precision);
  /* The three tangent-point parameters are the ones the encoding examples
   * of the standard use. `crs` names the geographic CRS they are given in,
   * which the examples leave implicit: EPSG:4979, the three-dimensional
   * WGS-84 code, since the tangent point carries a height. */
  char params[256];
  snprintf(params, sizeof(params),
    "longitude=%s&latitude=%s&height=%s&crs=EPSG:%d",
    blon, blat, bh, GEOPOSE_SRID_WGS84_3D);
  return geopose_frame_spec(id, params);
}

/**
 * @brief Build the Advanced-class GeoPose JSON object for a single pose.
 * @details Requirement 17, titled *Expression of outer frame*, makes
 * `Advanced.frameSpecification` an explicit outer frame, and the class has no
 * `position` member of its own, so the placement travels in that frame. The
 * frame written here is the LTP-ENU frame at the pose's own position, which
 * puts the pose at the origin of the frame it names. Its quaternion is then
 * the orientation in that frame, unrotated, and the document carries exactly
 * what the Basic-Quaternion document of the same pose carries.
 * @return On error return @p NULL
 */
static json_object *
pose_to_geopose_advanced(const Pose *pose, int precision)
{
  double lon, lat, h, W, X, Y, Z;
  if (! geopose_pose_components(pose, &lon, &lat, &h, &W, &X, &Y, &Z))
    return NULL;

  GeoPoseAnchor anchor;
  geopose_anchor_set(&anchor, GEOPOSE_DEG2RAD(lat), GEOPOSE_DEG2RAD(lon), h);

  json_object *root = json_object_new_object();
  json_object_object_add(root, "frameSpecification",
    geopose_outer_frame(GEOPOSE_ID_OUTER_FRAME, &anchor, precision));
  json_object *jq = json_object_new_object();
  json_object_object_add(jq, "x", geopose_new_double(X, precision));
  json_object_object_add(jq, "y", geopose_new_double(Y, precision));
  json_object_object_add(jq, "z", geopose_new_double(Z, precision));
  json_object_object_add(jq, "w", geopose_new_double(W, precision));
  json_object_object_add(root, "quaternion", jq);
  return root;
}

/**
 * @brief Set the tangent point of a composite from the pose of its first
 * instant.
 * @details Requirements 26 and 31 make the outer frame the first frame of a
 * series, and Requirement 34 the first frame of a stream, so one instant
 * fixes the frame that every later pose is expressed against.
 * @return On error return false
 */
static bool
geopose_anchor_from_instant(const TInstant *inst, GeoPoseAnchor *anchor)
{
  double lon, lat, h, W, X, Y, Z;
  if (! geopose_pose_components(DatumGetPoseP(tinstant_value_p(inst)),
      &lon, &lat, &h, &W, &X, &Y, &Z))
    return false;
  geopose_anchor_set(anchor, GEOPOSE_DEG2RAD(lat), GEOPOSE_DEG2RAD(lon), h);
  return true;
}

/**
 * @brief Build a SeriesHeader or SeriesTrailer `poseCount` bearing object.
 * @details `integrityCheck` is optional in both and is not emitted: the
 * standard leaves the digest input undefined, so any value this
 * implementation chose would not be checkable by another one.
 */
static json_object *
geopose_series_trailer(int count)
{
  json_object *res = json_object_new_object();
  json_object_object_add(res, "poseCount", json_object_new_int(count));
  return res;
}

static json_object *
geopose_series_header(int count, TimestampTz start, TimestampTz stop,
  interpType interp)
{
  json_object *res = json_object_new_object();
  json_object_object_add(res, "poseCount", json_object_new_int(count));
  json_object_object_add(res, "startInstant",
    json_object_new_int64(geopose_instant_out(start)));
  json_object_object_add(res, "stopInstant",
    json_object_new_int64(geopose_instant_out(stop)));
  json_object_object_add(res, "transitionModel",
    geopose_transition_model(interp));
  return res;
}

/**
 * @brief Return the constant inter-pose duration of @p instants in
 * milliseconds, or -1 if the instants are not equally spaced by a whole
 * number of milliseconds.
 * @details A Regular Series states the spacing once, as an integer number
 * of milliseconds (Requirement 25), and carries no per-pose time. It can
 * therefore only represent instants that are equally spaced *and* whose
 * spacing is a whole number of milliseconds; anything else must go out as
 * an Irregular Series to stay lossless.
 */
static int64
geopose_interpose_duration(const TInstant **instants, int count)
{
  if (count < 2)
    return -1;
  int64 delta = (int64) instants[1]->t - (int64) instants[0]->t;
  if (delta <= 0 || delta % 1000 != 0)
    return -1;
  for (int i = 2; i < count; i++)
  {
    if ((int64) instants[i]->t - (int64) instants[i - 1]->t != delta)
      return -1;
  }
  return delta / 1000;
}

/**
 * @brief Build a Composite Sequence Series document for a temporal pose
 * @param[in] temp Temporal pose
 * @param[in] regular True to emit a Regular Series, false for an Irregular
 * one
 * @param[in] precision Significant digits in the emitted numbers
 * @return On error return @p NULL
 */
static json_object *
tpose_to_geopose_series(const Temporal *temp, bool regular, int precision)
{
  int count;
  const TInstant **instants = temporal_insts_p(temp, &count);
  if (instants == NULL)
    return NULL;
  int64 duration = geopose_interpose_duration(instants, count);
  if (regular && duration < 0)
  {
    pfree(instants);
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "A GeoPose Regular Series requires at least two instants equally "
      "spaced by a whole number of milliseconds");
    return NULL;
  }

  GeoPoseAnchor anchor;
  if (! geopose_anchor_from_instant(instants[0], &anchor))
  {
    pfree(instants);
    return NULL;
  }

  json_object *arr = json_object_new_array();
  for (int i = 0; i < count; i++)
  {
    json_object *frame = geopose_inner_frame(GEOPOSE_ID_INNER_FRAME, &anchor,
      DatumGetPoseP(tinstant_value_p(instants[i])), precision);
    if (frame == NULL)
    {
      json_object_put(arr);
      pfree(instants);
      return NULL;
    }
    if (regular)
      json_object_array_add(arr, frame);
    else
    {
      json_object *elem = json_object_new_object();
      json_object_object_add(elem, "frame", frame);
      json_object_object_add(elem, "validTime",
        json_object_new_int64(geopose_instant_out(instants[i]->t)));
      json_object_array_add(arr, elem);
    }
  }

  json_object *root = json_object_new_object();
  json_object_object_add(root, "header", geopose_series_header(count,
    instants[0]->t, instants[count - 1]->t,
    MEOS_FLAGS_GET_INTERP(temp->flags)));
  if (regular)
    json_object_object_add(root, "interPoseDuration",
      json_object_new_int64(duration));
  json_object_object_add(root, "outerFrame",
    geopose_outer_frame(GEOPOSE_ID_OUTER_FRAME, &anchor, precision));
  json_object_object_add(root, regular ? "innerFrameSeries" :
    "innerFrameAndTimeSeries", arr);
  json_object_object_add(root, "trailer", geopose_series_trailer(count));
  pfree(instants);
  return root;
}

/*****************************************************************************
 * OGC GeoPose Composite Sequence classes — JSON decoding
 *****************************************************************************/

/**
 * @brief Return the `parameters` string of a FrameSpecification object,
 * checking that the object carries the three members the schema requires.
 * @return On error return @p NULL
 */
static const char *
geopose_frame_parameters(json_object *frame, const char *what)
{
  if (frame == NULL || ! json_object_is_type(frame, json_type_object))
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "GeoPose series %s must be a FrameSpecification object", what);
    return NULL;
  }
  json_object *jpar = geopose_find_member(frame, "parameters");
  if (jpar == NULL || ! json_object_is_type(jpar, json_type_string))
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "GeoPose series %s missing required 'parameters' string", what);
    return NULL;
  }
  return json_object_get_string(jpar);
}

/**
 * @brief Build the outer-frame anchor from a series' `outerFrame` member.
 * @return On error return false
 */
static bool
geopose_anchor_from_json(json_object *root, GeoPoseAnchor *anchor)
{
  const char *params = geopose_frame_parameters(
    geopose_find_member(root, "outerFrame"), "'outerFrame'");
  if (params == NULL)
    return false;
  double lon, lat, h;
  if (! geopose_param_number(geopose_param_find(params, "longitude"), &lon) ||
      ! geopose_param_number(geopose_param_find(params, "latitude"), &lat) ||
      ! geopose_param_number(geopose_param_find(params, "height"), &h))
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "GeoPose series 'outerFrame' parameters must carry 'longitude', "
      "'latitude' and 'height'");
    return false;
  }
  geopose_anchor_set(anchor, GEOPOSE_DEG2RAD(lat), GEOPOSE_DEG2RAD(lon), h);
  return true;
}

/**
 * @brief Build the pose of an inner FrameSpecification of a series.
 * @return On error return @p NULL
 */
static Pose *
geopose_pose_from_inner_frame(const GeoPoseAnchor *anchor, json_object *frame)
{
  const char *params = geopose_frame_parameters(frame, "inner frame");
  if (params == NULL)
    return NULL;
  double t[3], r[4];
  if (! geopose_param_list(geopose_param_find(params, "translation"), 3, t) ||
      ! geopose_param_list(geopose_param_find(params, "rotation"), 4, r))
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "GeoPose series inner frame parameters must carry a 3-element "
      "'translation' and a 4-element 'rotation'");
    return NULL;
  }
  double lat_rad, lon_rad, h;
  geopose_anchor_position(anchor, t[0], t[1], t[2], &lat_rad, &lon_rad, &h);
  /* Undo the tangent-plane convergence rotation applied on output. */
  double rw, rx, ry, rz, W, X, Y, Z;
  geopose_anchor_rotation(anchor, lat_rad, lon_rad, &rw, &rx, &ry, &rz);
  pose_quaternion_mul(rw, -rx, -ry, -rz, r[0], r[1], r[2], r[3],
    &W, &X, &Y, &Z);
  return pose_make_3d(GEOPOSE_RAD2DEG(lon_rad), GEOPOSE_RAD2DEG(lat_rad), h,
    W, X, Y, Z, true, GEOPOSE_GEOGRAPHIC_SRID);
}

/**
 * @brief Build a temporal pose from a Composite Sequence Series document
 * @details A series carries no bounds inclusivity and no gaps, so it always
 * reads back as a single closed sequence, or as an instant when the series
 * holds one pose and interpolates nothing.
 * @return On error return @p NULL
 */
static Temporal *
tpose_from_geopose_series(json_object *root, json_object *elements,
  bool regular)
{
  GeoPoseAnchor anchor;
  if (! geopose_anchor_from_json(root, &anchor))
    return NULL;
  if (! json_object_is_type(elements, json_type_array))
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "GeoPose series inner frame member must be a JSON array");
    return NULL;
  }
  int count = (int) json_object_array_length(elements);
  if (count == 0)
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "GeoPose series inner frame array must hold at least one element");
    return NULL;
  }

  json_object *header = geopose_find_member(root, "header");
  if (header == NULL || ! json_object_is_type(header, json_type_object))
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "GeoPose series missing required 'header' object");
    return NULL;
  }
  json_object *jstart = geopose_find_member(header, "startInstant");
  if (jstart == NULL || ! json_object_is_type(jstart, json_type_int))
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "GeoPose series header missing required integer 'startInstant'");
    return NULL;
  }
  TimestampTz start = geopose_instant_in(json_object_get_int64(jstart));
  interpType interp = geopose_transition_model_interp(
    geopose_find_member(header, "transitionModel"));

  /* A Regular Series states the spacing once instead of per pose. */
  int64 duration = 0;
  if (regular)
  {
    json_object *jdur = geopose_find_member(root, "interPoseDuration");
    if (jdur == NULL || ! json_object_is_type(jdur, json_type_int))
    {
      meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
        "GeoPose Regular Series missing required integer "
        "'interPoseDuration'");
      return NULL;
    }
    duration = json_object_get_int64(jdur);
  }

  TInstant **instants = palloc(sizeof(TInstant *) * count);
  int nfilled = 0;
  for (int i = 0; i < count; i++)
  {
    json_object *elem = json_object_array_get_idx(elements, i);
    json_object *frame;
    TimestampTz t;
    if (regular)
    {
      frame = elem;
      t = start + (TimestampTz) (duration * i * 1000);
    }
    else
    {
      if (! json_object_is_type(elem, json_type_object))
      {
        meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
          "GeoPose Irregular Series element must be a FrameAndTime object");
        break;
      }
      frame = geopose_find_member(elem, "frame");
      json_object *jt = geopose_find_member(elem, "validTime");
      if (jt == NULL || ! json_object_is_type(jt, json_type_int))
      {
        meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
          "GeoPose Irregular Series element missing required integer "
          "'validTime'");
        break;
      }
      t = geopose_instant_in(json_object_get_int64(jt));
    }
    Pose *pose = geopose_pose_from_inner_frame(&anchor, frame);
    if (pose == NULL)
      break;
    instants[nfilled++] = tinstant_make(PointerGetDatum(pose), T_TPOSE, t);
    pfree(pose);
  }
  if (nfilled < count)
  {
    pfree_array((void **) instants, nfilled);
    return NULL;
  }

  Temporal *result;
  if (count == 1 && interp != LINEAR && interp != STEP)
  {
    result = (Temporal *) instants[0];
    pfree(instants);
  }
  else
  {
    result = (Temporal *) tsequence_make(instants, count, true, true, interp,
      NORMALIZE);
    pfree_array((void **) instants, count);
  }
  return result;
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


/**
 * @brief Build the single-pose document of a temporal instant.
 * @details The Basic classes carry no time, so the instant's own is added
 * under the name `validTime` that the Advanced, Chain, Graph, Series and
 * Stream classes all use, and with the type they all give it: a
 * GeoPose_Instant, that is Unix time in integer milliseconds. Every time
 * this module writes is that same kind, whichever class the document
 * belongs to.
 * @return On error return @p NULL
 */
static char *
tposeinst_as_geopose(const TInstant *inst, int conformance, int precision)
{
  const Pose *pose = DatumGetPoseP(tinstant_value_p(inst));
  json_object *root = pose_to_geopose_object(pose, conformance, precision);
  if (root == NULL)
    return NULL;
  json_object_object_add(root, "validTime",
    json_object_new_int64(geopose_instant_out(inst->t)));
  char *res = pstrdup(json_object_to_json_string_ext(root,
    GEOPOSE_JSON_FLAGS));
  json_object_put(root);
  return res;
}

/**
 * @ingroup meos_pose_geopose_accessor
 * @brief Return the OGC GeoPose JSON representation of a temporal pose
 * @details The class follows from the value, not from an argument. A
 * @p TInstant is a single-pose document carrying the instant's @p validTime; a
 * @p TSequence or @p TSequenceSet is a Composite Sequence Series, the
 * Regular one when its instants are equally spaced by a whole number of
 * milliseconds and the Irregular one otherwise. The outer frame of a Series
 * is the LTP-ENU frame at the first pose and each inner frame is that pose's
 * translation and rotation relative to it.
 *
 * A Series flattens a @p TSequenceSet and drops bounds inclusivity, the
 * standard's model having neither gaps nor open bounds. @p conformance
 * chooses the class of a single-pose document and has no effect on
 * a Series, whose inner frames carry a quaternion and offer no choice.
 * @param[in] temp Temporal pose
 * @param[in] conformance Class of a single-pose document
 * (0 = Basic-Quaternion, 1 = Basic-YPR, 2 = Advanced)
 * @param[in] precision Significant digits in JSON numbers; -1 = lossless
 * @return On error return @p NULL
 * @csqlfn #Tpose_as_geopose()
 */
char *
tpose_as_geopose(const Temporal *temp, int conformance, int precision)
{
  VALIDATE_TPOSE(temp, NULL);

  /* The target class follows from the value. A single pose is a Basic
   * document; a value that evolves over time is a Composite Sequence
   * Series, Regular when its instants are equally spaced and Irregular
   * otherwise. */
  if (temp->subtype == TINSTANT)
    return tposeinst_as_geopose((const TInstant *) temp, conformance,
      precision);

  int count;
  const TInstant **instants = temporal_insts_p(temp, &count);
  if (instants == NULL)
    return NULL;
  bool regular = (geopose_interpose_duration(instants, count) >= 0);
  pfree(instants);
  json_object *series = tpose_to_geopose_series(temp, regular, precision);
  if (series == NULL)
    return NULL;
  char *res = pstrdup(json_object_to_json_string_ext(series,
    GEOPOSE_JSON_FLAGS));
  json_object_put(series);
  return res;
}

/**
 * @ingroup meos_pose_base_geopose
 * @brief Return the OGC GeoPose StreamHeader of a temporal pose
 * @details A Stream is the open-ended member of the Composite Sequence
 * classes: it carries the same frames as an Irregular Series but states
 * neither how many poses there are nor when they end, since more may
 * arrive. The standard splits it into two documents, and this is the one
 * that "appears once at the beginning of a stream": the transition model
 * of Requirement 35 and, per Requirement 34, the outer frame that every
 * element is expressed against.
 *
 * The frame is anchored at the first pose of @p temp, so a producer that
 * accumulates its value with @p temporal_append_tinstant writes the header
 * from the same value it goes on to stream, and the elements it emits speak
 * of the same tangent point.
 * @param[in] temp Temporal pose
 * @param[in] precision Significant digits in JSON numbers; -1 = lossless
 * @return On error return @p NULL
 */
static json_object *
geopose_stream_header_obj(const Temporal *temp, const GeoPoseAnchor *anchor,
  int precision)
{
  json_object *res = json_object_new_object();
  json_object_object_add(res, "transitionModel",
    geopose_transition_model(MEOS_FLAGS_GET_INTERP(temp->flags)));
  json_object_object_add(res, "outerFrame",
    geopose_outer_frame(GEOPOSE_ID_OUTER_FRAME, anchor, precision));
  return res;
}

/**
 * @brief Build the `StreamElement` of one instant against an anchored outer
 * frame. Returns @p NULL on error.
 */
static json_object *
geopose_stream_element_obj(const GeoPoseAnchor *anchor, const TInstant *inst,
  int precision)
{
  json_object *frame = geopose_inner_frame(GEOPOSE_ID_INNER_FRAME, anchor,
    DatumGetPoseP(tinstant_value_p(inst)), precision);
  if (frame == NULL)
    return NULL;
  json_object *fat = json_object_new_object();
  json_object_object_add(fat, "frame", frame);
  json_object_object_add(fat, "validTime",
    json_object_new_int64(geopose_instant_out(inst->t)));
  json_object *res = json_object_new_object();
  json_object_object_add(res, "streamElement", fat);
  return res;
}

char *
tpose_as_geopose_stream_header(const Temporal *temp, int precision)
{
  VALIDATE_TPOSE(temp, NULL);

  const TInstant *first = temporal_start_inst(temp);
  if (first == NULL)
    return NULL;
  GeoPoseAnchor anchor;
  if (! geopose_anchor_from_instant(first, &anchor))
    return NULL;

  json_object *root = geopose_stream_header_obj(temp, &anchor, precision);
  char *res = pstrdup(json_object_to_json_string_ext(root,
    GEOPOSE_JSON_FLAGS));
  json_object_put(root);
  return res;
}

/**
 * @ingroup meos_pose_base_geopose
 * @brief Return the OGC GeoPose StreamElement of one instant of a temporal
 * pose
 * @details This is "the repeated information streamed at irregular times":
 * the inner frame of @p inst and the instant it is valid at, wrapped as
 * Requirement 36 asks. The frame is relative to the outer frame of the
 * stream, which @p temp anchors at its first pose, so the same value that
 * produced the header produces every element and the two agree.
 * @param[in] temp Temporal pose the stream is written from
 * @param[in] inst Instant to write
 * @param[in] precision Significant digits in JSON numbers; -1 = lossless
 * @return On error return @p NULL
 */
char *
tpose_as_geopose_stream_element(const Temporal *temp, const TInstant *inst,
  int precision)
{
  VALIDATE_TPOSE(temp, NULL);
  VALIDATE_NOT_NULL(inst, NULL);

  const TInstant *first = temporal_start_inst(temp);
  if (first == NULL)
    return NULL;
  GeoPoseAnchor anchor;
  if (! geopose_anchor_from_instant(first, &anchor))
    return NULL;

  json_object *root = geopose_stream_element_obj(&anchor, inst, precision);
  if (root == NULL)
    return NULL;
  char *res = pstrdup(json_object_to_json_string_ext(root,
    GEOPOSE_JSON_FLAGS));
  json_object_put(root);
  return res;
}

/**
 * @ingroup meos_pose_base_geopose
 * @brief Return the OGC GeoPose Stream of a temporal pose
 * @details A stream written as one document: the `StreamHeader` that opens
 * it, and every `StreamElement` it carries, as
 * `GeoPose.Composite.Sequence.Stream.Schema.json` requires. The two
 * incremental entry points write the same documents a piece at a time, for a
 * producer emitting poses as they arrive; this writes the stream a reader
 * already holds whole, which is what a query returns and what a conformance
 * submission carries.
 *
 * The outer frame is anchored at the first pose of @p temp, so the header and
 * every element speak of one tangent point, exactly as the incremental pair
 * does.
 * @param[in] temp Temporal pose
 * @param[in] precision Significant digits in JSON numbers; -1 = lossless
 * @return On error return @p NULL
 * @csqlfn #Tpose_as_geopose_stream()
 */
char *
tpose_as_geopose_stream(const Temporal *temp, int precision)
{
  VALIDATE_TPOSE(temp, NULL);

  int count;
  const TInstant **instants = temporal_insts_p(temp, &count);
  if (instants == NULL)
    return NULL;
  GeoPoseAnchor anchor;
  if (! geopose_anchor_from_instant(instants[0], &anchor))
  {
    pfree(instants);
    return NULL;
  }

  json_object *arr = json_object_new_array();
  for (int i = 0; i < count; i++)
  {
    json_object *elem = geopose_stream_element_obj(&anchor, instants[i],
      precision);
    if (elem == NULL)
    {
      json_object_put(arr);
      pfree(instants);
      return NULL;
    }
    json_object_array_add(arr, elem);
  }

  json_object *root = json_object_new_object();
  json_object_object_add(root, "header",
    geopose_stream_header_obj(temp, &anchor, precision));
  json_object_object_add(root, "streamElements", arr);
  char *res = pstrdup(json_object_to_json_string_ext(root,
    GEOPOSE_JSON_FLAGS));
  json_object_put(root);
  pfree(instants);
  return res;
}

/**
 * @brief Parse one element of an @p instants array (single-pose object
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
  /* `validTime` is a GeoPose_Instant, and a datetime string is taken as
   * well since that is what the MobilityDB envelope holds. */
  json_object *jt = geopose_find_member(obj, "validTime");
  TimestampTz t;
  if (jt != NULL && json_object_is_type(jt, json_type_int))
    t = geopose_instant_in(json_object_get_int64(jt));
  else if (jt != NULL && json_object_is_type(jt, json_type_string))
    t = pg_timestamptz_in(json_object_get_string(jt), -1);
  else
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "A single-pose GeoPose document needs a 'validTime' to read as a "
      "temporal pose");
    return NULL;
  }
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
 * @details The document shape is auto-detected:
 *
 *   - An OGC Composite Sequence Series, recognized by its
 *     @p innerFrameAndTimeSeries or @p innerFrameSeries member. The
 *     interpolation comes from the header's transition model and the
 *     result is one closed sequence, or an instant for a single
 *     non-interpolated pose.
 *   - The MobilityDB @p TemporalGeoPose envelope, whose per-instant
 *     payload is auto-detected as Basic-Quaternion or Basic-YPR by the
 *     same rule as the static entry point, and whose interpolation and
 *     bounds-inclusion flags come from the envelope.
 *
 * The resulting temporal pose has SRID 4326.
 * @param[in] json GeoPose JSON string
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

  /* A single-pose document holds one pose. With a `validTime` it is a
   * temporal instant, which is what a temporal pose of that subtype writes.
   * A Basic document is recognized by its `position` and an Advanced one by
   * the frame it carries it in; a Series names its outer frame by a member of
   * its own, so neither is mistaken for the other. */
  if (geopose_find_member(root, "position") != NULL ||
      geopose_find_member(root, "frameSpecification") != NULL)
  {
    TInstant *inst = tposeinst_from_geopose_object(root);
    json_object_put(root);
    return (Temporal *) inst;
  }

  /* An OGC Composite Sequence Series is recognized by the inner frame
   * member that its conformance class requires. */
  json_object *jirr = geopose_find_member(root, "innerFrameAndTimeSeries");
  json_object *jreg = geopose_find_member(root, "innerFrameSeries");
  if (jirr != NULL || jreg != NULL)
  {
    Temporal *result = tpose_from_geopose_series(root,
      (jirr != NULL) ? jirr : jreg, jirr == NULL);
    json_object_put(root);
    return result;
  }

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

#if POSE
/*****************************************************************************
 * OGC GeoPose Composite Chain class
 *
 * A Chain document names an outer frame and a sequence of transformations
 * reaching a final innermost frame, which is what a pose chain holds: its
 * outer link is placed in a topocentric frame and every later link is a rigid
 * transformation read in the axes of the link before it.
 *****************************************************************************/

/**
 * @brief Build the FrameSpecification of a link that is read in the axes of
 * the link before it
 * @details Only the outer link names a frame, so every later link travels as
 * the translation and rotation it stores, with no geodesy applied.
 */
static json_object *
geopose_chain_link_frame(const Pose *pose, int precision)
{
  bool hasz = MEOS_FLAGS_GET_Z(pose->flags);
  double x = pose->data[0], y = pose->data[1], z = hasz ? pose->data[2] : 0.0;
  double W, X, Y, Z;
  if (hasz)
  {
    W = pose->data[3]; X = pose->data[4];
    Y = pose->data[5]; Z = pose->data[6];
  }
  else
    /* A planar link turns about the vertical axis by its stored angle */
    pose_ypr_to_quaternion(pose->data[2], 0.0, 0.0, &W, &X, &Y, &Z);
  char bx[64], by[64], bz[64], bw[64], bqx[64], bqy[64], bqz[64];
  geopose_str_double(bx, sizeof(bx), x, precision);
  geopose_str_double(by, sizeof(by), y, precision);
  geopose_str_double(bz, sizeof(bz), z, precision);
  geopose_str_double(bw, sizeof(bw), W, precision);
  geopose_str_double(bqx, sizeof(bqx), X, precision);
  geopose_str_double(bqy, sizeof(bqy), Y, precision);
  geopose_str_double(bqz, sizeof(bqz), Z, precision);
  char params[512];
  snprintf(params, sizeof(params),
    "translation=[%s, %s, %s]&rotation=[%s, %s, %s, %s]",
    bx, by, bz, bw, bqx, bqy, bqz);
  return geopose_frame_spec(GEOPOSE_ID_CHAIN_INNER_FRAME, params);
}

/**
 * @ingroup meos_posechain_inout
 * @brief Return the OGC GeoPose Composite Chain JSON representation of a
 * temporal pose chain
 * @details The document carries the valid time of the instant, the LTP-ENU
 * frame tangent at the outer link's position, and one transformation per
 * link. The first of them takes that tangent frame to the outer link's own
 * frame; each later one is the link as it is stored, read in the axes of its
 * parent.
 * @param[in] temp Temporal pose chain holding a single instant
 * @param[in] precision Maximum number of decimal digits
 * @return On error return @p NULL
 * @csqlfn #Tposechain_as_geopose()
 */
char *
tposechain_as_geopose(const Temporal *temp, int precision)
{
  VALIDATE_TPOSECHAIN(temp, NULL);
  if (temp->subtype != TINSTANT)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "A GeoPose Chain document carries one valid time, so it is written "
      "from a single instant; use atTime to obtain one");
    return NULL;
  }
  const TInstant *inst = (const TInstant *) temp;
  const PoseChain *pc = DatumGetPoseChainP(tinstant_value_p(inst));
  int count = posechain_num_poses(pc);
  if (count < 2)
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "A GeoPose Chain frame chain holds at least two frames, and the pose "
      "chain holds %d", count);
    return NULL;
  }
  Pose *outer = posechain_pose_n(pc, 1);
  if (outer == NULL)
    return NULL;
  GeoPoseAnchor anchor;
  double lon, lat, h, W, X, Y, Z;
  if (! geopose_pose_components(outer, &lon, &lat, &h, &W, &X, &Y, &Z))
  {
    pfree(outer);
    return NULL;
  }
  geopose_anchor_set(&anchor, GEOPOSE_DEG2RAD(lat), GEOPOSE_DEG2RAD(lon), h);

  json_object *root = json_object_new_object();
  json_object_object_add(root, "validTime",
    json_object_new_int64(geopose_instant_out(inst->t)));
  json_object_object_add(root, "outerFrame",
    geopose_outer_frame(GEOPOSE_ID_CHAIN_OUTER_FRAME, &anchor, precision));
  json_object *chain = json_object_new_array();
  json_object_array_add(chain,
    geopose_inner_frame(GEOPOSE_ID_CHAIN_INNER_FRAME, &anchor, outer,
      precision));
  pfree(outer);
  for (int i = 2; i <= count; i++)
  {
    Pose *link = posechain_pose_n(pc, i);
    if (link == NULL)
    {
      json_object_put(root); json_object_put(chain);
      return NULL;
    }
    json_object_array_add(chain, geopose_chain_link_frame(link, precision));
    pfree(link);
  }
  json_object_object_add(root, "frameChain", chain);
  char *result = pstrdup(json_object_to_json_string_ext(root,
    GEOPOSE_JSON_FLAGS));
  json_object_put(root);
  return result;
}

/**
 * @ingroup meos_posechain_inout
 * @brief Return the GeoPose Graph representation of an array of temporal pose
 * chains
 * @details A graph of frames is a set of pose chains sharing their outermost
 * frame. The frame list holds that one topocentric frame followed by the links
 * of every chain, and the transform list names the parent and the child of
 * each edge by their position in that list: an edge from the outermost frame
 * to the first link of a chain, and one between each pair of links after it.
 * The edges carry no transformation, which lives in the frames they name.
 * @param[in] temparr Array of temporal pose chains, each of a single instant
 * @param[in] count Number of elements in the array
 * @param[in] precision Maximum number of decimal digits
 * @return On error return @p NULL
 * @csqlfn #Tposechainarr_as_geopose()
 */
char *
tposechainarr_as_geopose(const Temporal **temparr, int count, int precision)
{
  VALIDATE_NOT_NULL(temparr, NULL);
  if (! ensure_positive(count))
    return NULL;

  /* A graph carries one valid time, so every chain is read at one instant and
   * they agree on which */
  TimestampTz t = 0;
  for (int i = 0; i < count; i++)
  {
    const Temporal *temp = temparr[i];
    VALIDATE_TPOSECHAIN(temp, NULL);
    if (temp->subtype != TINSTANT)
    {
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "A GeoPose Graph document carries one valid time, so it is written "
        "from single instants; use atTime to obtain one");
      return NULL;
    }
    TimestampTz ti = ((const TInstant *) temp)->t;
    if (i == 0)
      t = ti;
    else if (ti != t)
    {
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
        "The pose chains of a GeoPose Graph are read at one instant");
      return NULL;
    }
  }

  /* An inner frame cannot be topocentric, so a graph has ONE topocentric
   * frame and every chain hangs from it: it is anchored at the outermost link
   * of the first chain */
  const PoseChain *first = DatumGetPoseChainP(
    tinstant_value_p((const TInstant *) temparr[0]));
  Pose *outer = posechain_pose_n(first, 1);
  if (outer == NULL)
    return NULL;
  GeoPoseAnchor anchor;
  double lon, lat, h, W, X, Y, Z;
  if (! geopose_pose_components(outer, &lon, &lat, &h, &W, &X, &Y, &Z))
  {
    pfree(outer);
    return NULL;
  }
  geopose_anchor_set(&anchor, GEOPOSE_DEG2RAD(lat), GEOPOSE_DEG2RAD(lon), h);
  pfree(outer);

  json_object *frames = json_object_new_array();
  json_object *transforms = json_object_new_array();
  json_object_array_add(frames,
    geopose_outer_frame(GEOPOSE_ID_CHAIN_OUTER_FRAME, &anchor, precision));

  /* The topocentric frame occupies position 0 and the links follow it */
  int next = 1;
  for (int i = 0; i < count; i++)
  {
    const PoseChain *pc = DatumGetPoseChainP(
      tinstant_value_p((const TInstant *) temparr[i]));
    int nlinks = posechain_num_poses(pc);
    int parent = 0;
    for (int j = 1; j <= nlinks; j++)
    {
      Pose *link = posechain_pose_n(pc, j);
      if (link == NULL)
      {
        json_object_put(frames);
        json_object_put(transforms);
        return NULL;
      }
      /* The outermost link is read against the topocentric frame and every
       * later one against the link before it */
      json_object_array_add(frames, (j == 1) ?
        geopose_inner_frame(GEOPOSE_ID_CHAIN_INNER_FRAME, &anchor, link,
          precision) :
        geopose_chain_link_frame(link, precision));
      pfree(link);
      json_object *pair = json_object_new_object();
      json_object *edge = json_object_new_array();
      json_object_array_add(edge, json_object_new_int(parent));
      json_object_array_add(edge, json_object_new_int(next));
      json_object_object_add(pair, "link", edge);
      json_object_array_add(transforms, pair);
      parent = next;
      next++;
    }
  }

  /* The frame list holds at least the two frames the class asks for: a pose
   * chain carries at least one link, so the topocentric frame and the first
   * link of the first chain already make two */
  json_object *root = json_object_new_object();
  json_object_object_add(root, "validTime",
    json_object_new_int64(geopose_instant_out(t)));
  json_object_object_add(root, "frameList", frames);
  json_object_object_add(root, "transformList", transforms);
  char *result = pstrdup(json_object_to_json_string_ext(root,
    GEOPOSE_JSON_FLAGS));
  json_object_put(root);
  return result;
}

/**
 * @brief Build the pose of a link read in the axes of the link before it
 * @return On error return @p NULL
 */
static Pose *
geopose_chain_link_pose(json_object *frame)
{
  const char *params = geopose_frame_parameters(frame, "frame chain element");
  if (params == NULL)
    return NULL;
  double t[3], r[4];
  if (! geopose_param_list(geopose_param_find(params, "translation"), 3, t) ||
      ! geopose_param_list(geopose_param_find(params, "rotation"), 4, r))
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "GeoPose Chain frame chain element parameters must carry a 3-element "
      "'translation' and a 4-element 'rotation'");
    return NULL;
  }
  return pose_make_3d(t[0], t[1], t[2], r[0], r[1], r[2], r[3], false,
    SRID_UNKNOWN);
}

/**
 * @ingroup meos_posechain_inout
 * @brief Return a temporal pose chain from an OGC GeoPose Composite Chain
 * JSON document
 * @param[in] json GeoPose Chain document
 * @return On error return @p NULL
 * @csqlfn #Tposechain_from_geopose()
 */
Temporal *
tposechain_from_geopose(const char *json)
{
  VALIDATE_NOT_NULL(json, NULL);
  json_tokener *tok = json_tokener_new();
  json_object *root = json_tokener_parse_ex(tok, json, -1);
  enum json_tokener_error err = json_tokener_get_error(tok);
  json_tokener_free(tok);
  if (root == NULL || err != json_tokener_success)
  {
    if (root != NULL) json_object_put(root);
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "Cannot parse the GeoPose Chain document");
    return NULL;
  }
  json_object *jtime = geopose_find_member(root, "validTime");
  json_object *jchain = geopose_find_member(root, "frameChain");
  if (jtime == NULL || ! json_object_is_type(jtime, json_type_int) ||
      jchain == NULL || ! json_object_is_type(jchain, json_type_array))
  {
    json_object_put(root);
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "A GeoPose Chain document carries an integer 'validTime' and an array "
      "'frameChain'");
    return NULL;
  }
  int count = (int) json_object_array_length(jchain);
  if (count < 2)
  {
    json_object_put(root);
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "A GeoPose Chain frame chain holds at least two frames, and the "
      "document carries %d", count);
    return NULL;
  }
  GeoPoseAnchor anchor;
  if (! geopose_anchor_from_json(root, &anchor))
  {
    json_object_put(root);
    return NULL;
  }
  Pose **poses = palloc(sizeof(Pose *) * count);
  int nposes = 0;
  bool failed = false;
  for (int i = 0; i < count; i++)
  {
    json_object *frame = json_object_array_get_idx(jchain, i);
    Pose *pose = (i == 0) ?
      geopose_pose_from_inner_frame(&anchor, frame) :
      geopose_chain_link_pose(frame);
    if (pose == NULL) { failed = true; break; }
    poses[nposes++] = pose;
  }
  Temporal *result = NULL;
  if (! failed)
  {
    PoseChain *pc = posechain_make((const Pose **) poses, count);
    if (pc != NULL)
    {
      result = (Temporal *) tinstant_make_free(PointerGetDatum(pc),
        T_TPOSECHAIN, geopose_instant_in(json_object_get_int64(jtime)));
    }
  }
  for (int i = 0; i < nposes; i++)
    pfree(poses[i]);
  pfree(poses);
  json_object_put(root);
  return result;
}
#endif /* POSE */

/*****************************************************************************/
