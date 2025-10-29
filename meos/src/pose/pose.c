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
 * @brief Basic functions for static pose objects.
 */

/* C */
#include <math.h>
#include <float.h>
#include <limits.h>
/* Postgres */
#include <postgres.h>
#include <pgtypes.h>
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif
#include <common/hashfn.h>
#include <utils/float.h>
/* MEOS */
#include <meos.h>
#include <meos_pose.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
#include <pgtypes.h>
#include "temporal/set.h"
#include "temporal/tsequence.h"
#include "temporal/type_inout.h"
#include "temporal/type_parser.h"
#include "temporal/type_util.h"
#include "geo/meos_transform.h"
#include "geo/tgeo_spatialfuncs.h"
#include "geo/tspatial.h"
#include "geo/tspatial_parser.h"
#include "pose/pose.h"

/** Buffer size for input and output of pose values */
#define MAXPOSELEN    256

/*****************************************************************************
 * Validity functions
 *****************************************************************************/

/**
 * @brief Ensure the validity of a pose and a geometry/geography
 */
bool
ensure_valid_pose_geo(const Pose *pose, const GSERIALIZED *gs)
{
  VALIDATE_NOT_NULL(pose, false); VALIDATE_NOT_NULL(gs, false); 
  if (gserialized_is_empty(gs) ||
      ! ensure_same_srid(pose_srid(pose), gserialized_get_srid(gs)) ||
      MEOS_FLAGS_GET_Z(pose->flags) != FLAGS_GET_Z(gs->gflags))
    return false;
  return true;
}

/**
 * @brief Ensure the validity of a pose and a spatiotemporal box
 */
bool
ensure_valid_pose_stbox(const Pose *pose, const STBox *box)
{
  VALIDATE_NOT_NULL(pose, false); VALIDATE_NOT_NULL(box, false);
  if (! ensure_has_X(T_STBOX, box->flags) ||
      ! ensure_same_srid(pose_srid(pose), box->srid))
    return false;
  return true;
}

/**
 * @brief Ensure the validity of two circular poses
 */
bool
ensure_valid_pose_pose(const Pose *pose1, const Pose *pose2)
{
  VALIDATE_NOT_NULL(pose1, false); VALIDATE_NOT_NULL(pose2, false); 
  if (! ensure_same_srid(pose_srid(pose1), pose_srid(pose2)) ||
      MEOS_FLAGS_GET_Z(pose1->flags) != MEOS_FLAGS_GET_Z(pose2->flags))
    return false;
  return true;
}

/**
 * @brief Return true if a set and a pose are valid for set operations
 * @param[in] s Set
 * @param[in] pose Value
 */
bool
ensure_valid_poseset_pose(const Set *s, const Pose *pose)
{
  /* Ensure the validity of the arguments */
  VALIDATE_POSESET(s, false); VALIDATE_NOT_NULL(pose, false);
  if (! ensure_same_srid(spatialset_srid(s), pose_srid(pose)) ||
      MEOS_FLAGS_GET_Z(pose->flags) != MEOS_FLAGS_GET_Z(s->flags))
    return false;
  return true;
}

/*****************************************************************************
 * Interpolation function
 *****************************************************************************/

/**
 * @brief Return the pose value interpolated from the two poses and a ratio
 * @param[in] start,end Poses
 * @param[in] ratio Value in [0,1] representing the duration of the
 * timestamps associated to `p1` and `p2` divided by the duration
 * of the timestamps associated to `p1` and `p3`
 */
Pose *
posesegm_interpolate(const Pose *start, const Pose *end, double ratio)
{
  if (! ensure_valid_pose_pose(start, end))
    return NULL; 
  Pose *result;
  if (! MEOS_FLAGS_GET_Z(start->flags))
  {
    double x = start->data[0] * (1 - ratio) + end->data[0] * ratio;
    double y = start->data[1] * (1 - ratio) + end->data[1] * ratio;
    double theta;
    double theta_delta = end->data[2] - start->data[2];
    /* If fabs(theta_delta) == M_PI: Always turn counter-clockwise */
    if (fabs(theta_delta) < MEOS_EPSILON)
      theta = start->data[2];
    else if (theta_delta > 0 && fabs(theta_delta) <= M_PI)
      theta = start->data[2] + theta_delta * ratio;
    else if (theta_delta > 0 && fabs(theta_delta) > M_PI)
      theta = end->data[2] + (2 * M_PI - theta_delta) * (1 - ratio);
    else if (theta_delta < 0 && fabs(theta_delta) < M_PI)
      theta = start->data[2] + theta_delta * ratio;
    else /* (theta_delta < 0 && fabs(theta_delta) >= M_PI) */
      theta = start->data[2] + (2 * M_PI + theta_delta) * ratio;
    if (theta > M_PI)
      theta = theta - 2 * M_PI;
    result = pose_make_2d(x, y, theta, pose_srid(start));
  }
  else
  {
    double x = start->data[0] * (1 - ratio) + end->data[0] * ratio;
    double y = start->data[1] * (1 - ratio) + end->data[1] * ratio;
    double z = start->data[2] * (1 - ratio) + end->data[2] * ratio;
    double W, W1 = start->data[3], W2 = end->data[3];
    double X, X1 = start->data[4], X2 = end->data[4];
    double Y, Y1 = start->data[5], Y2 = end->data[5];
    double Z, Z1 = start->data[6], Z2 = end->data[6];
    double dot =  W1 * W2 + X1 * X2 + Y1 * Y2 + Z1 * Z2;
    if (dot < 0.0f)
    {
      W2 = -W2;
      X2 = -X2;
      Y2 = -Y2;
      Z2 = -Z2;
      dot = -dot;
    }
    const double DOT_THRESHOLD = 0.9995;
    if (dot > DOT_THRESHOLD)
    {
      W = W1 + (W2 - W1) * ratio;
      X = X1 + (X2 - X1) * ratio;
      Y = Y1 + (Y2 - Y1) * ratio;
      Z = Z1 + (Z2 - Z1) * ratio;
    }
    else
    {
      double theta_0 = acos(dot);
      double theta = theta_0 * ratio;
      double sin_theta = sin(theta);
      double sin_theta_0 = sin(theta_0);
      double s1 = cos(theta) - dot * sin_theta / sin_theta_0;
      double s2 = sin_theta / sin_theta_0;
      W = W1 * s1 + W2 * s2;
      X = X1 * s1 + X2 * s2;
      Y = Y1 * s1 + Y2 * s2;
      Z = Z1 * s1 + Z2 * s2;
    }
    double norm = W * W + X * X + Y * Y + Z * Z;
    W /= norm;
    X /= norm;
    Y /= norm;
    Z /= norm;
    result = pose_make_3d(x, y, z, W, X, Y, Z, pose_srid(start));
  }
  return result;
}

/**
 * @brief Return a float in (0,1) if a network point segment intersects a 
 * network point, return -1.0 if the network point is not located in the
 * segment or if it is approximately equal to the start or the end valuess
 * @param[in] start,end Values defining the segment
 * @param[in] value Value to locate
 * @note The function returns -1.0 if the network point is approximately equal 
 * to the start or the end network points since it is used in the lifting
 * infrastructure for determining the crossings or the turning points after
 * verifying that the bounds of the segment are not equal to the value.
 */
long double
posesegm_locate(const Pose *start, const Pose *end, const Pose *value)
{
   /* Return if the value to locate has a different road identifier */
  if (ensure_valid_pose_pose(start, end) || 
      ensure_valid_pose_pose(start, value))
    return -1.0;

  GSERIALIZED *gs1 = pose_to_point(start);
  GSERIALIZED *gs2 = pose_to_point(end);
  GSERIALIZED *gs = pose_to_point(value);
  long double result1 = -1.0;
  long double result2 = -1.0;
  if (! geopoint_eq(gs1, gs2))
  {
    result1 = pointsegm_locate(PointerGetDatum(gs1), PointerGetDatum(gs2),
      PointerGetDatum(gs), NULL);
    if (result1 < 0.0)
      return -1.0;
  }
  else
  {
    /* If constant segment and the point of the value is different */
    if (! geopoint_eq(gs1, gs))
      return -1.0;
  }
  if (MEOS_FLAGS_GET_Z(start->flags))
  {
    // TODO
      return -1.0;
  }
  else
  {
    double rotation1 = pose_rotation(start);
    double rotation2 = pose_rotation(value);
    double rotation = pose_rotation(value);
    if (rotation1 != rotation2)
    {
      result2 = floatsegm_locate(rotation1, rotation2, rotation);
      if (result2 < 0.0)
        return -1.0;
    }
    else
    {
      /* If the rotiation are equal return result1 where
       * - result1 = -1.0 if gs1 == gs2 == gs, or
       * - result1 in [0,1] if gs1 != gs2 */
      return result1;
    }
  }
  if (result1 >= 0.0 && result2 >= 0.0)
    return (fabsl(result1 - result2) <= MEOS_EPSILON) ? result1 : -1.0;
  if (result1 < 0.0 && result2 >= 0)
    return result2;
  else if(result1 >= 0 && result2 <= 0)
    return result1;
  else /* The three values are equal */
    return -1.0;
}

/*****************************************************************************
 * Collinear function
 *****************************************************************************/

/**
 * @brief Return true if the three values are collinear
 * @param[in] p1,p2,p3 Poses
 * @param[in] ratio Value in [0,1] representing the duration of the
 * timestamps associated to `p1` and `p2` divided by the duration
 * of the timestamps associated to `p1` and `p3`
 */
bool
pose_collinear(const Pose *p1, const Pose *p2, const Pose *p3, double ratio)
{
  assert(p1); assert(p2); assert(p3); 
  Pose *p2_interpolated = posesegm_interpolate(p1, p3, ratio);
  bool result = pose_same(p2, p2_interpolated);
  pfree(p2_interpolated);
  return result;
}

/*****************************************************************************
 * Parameter tests
 *****************************************************************************/

/**
 * @brief Ensure that a 3D orientation has a unit norm
 */
bool
ensure_valid_rotation(double theta)
{
  if (theta < -M_PI || theta > M_PI)
  {
    meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE,
      "Rotation angle must be in ]-pi, pi]. Received: %f", theta);
    return false;
  }
  return true;
}

/**
 * @brief Ensure that a 3D orientation has a unit norm
 */
bool
ensure_unit_norm(double W, double X, double Y, double Z)
{
  if (fabs(sqrt(W * W + X * X + Y * Y + Z * Z) - 1) > MEOS_EPSILON)
  {
    meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE,
      "Rotation quaternion must be of unit norm. Received: %f",
      sqrt(W * W + X * X + Y * Y + Z * Z));
    return false;
  }
  return true;
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/**
 * @brief Parse a pose value from the buffer
 */
Pose *
pose_parse(const char **str, bool end)
{
  Pose *result;
  const char *type_str = meostype_name(T_POSE);

  /* Determine whether the pose has an SRID */
  int32_t srid;
  srid_parse(str, &srid);

  /* Determine whether the pose has a geometry */
  int32_t srid_geo;
  GSERIALIZED *geo = NULL;
  if (strncasecmp(*str,"POSE",4) != 0)
  {
    if (! geo_parse(str, T_GEOMETRY, ',', &srid_geo, &geo))
      return NULL;
  }

  if (strncasecmp(*str, "POSE", 4) == 0)
  {
    *str += 4;
    p_whitespace(str);
  }
  else
  {
    meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
      "Could not parse %s value: Missing prefix 'Pose'",
      meostype_name(T_POSE));
    return NULL;
  }

  /* Parse opening parenthesis */
  if (! ensure_oparen(str, type_str))
    return NULL;

  /* Parse geo */
  p_whitespace(str);
  GSERIALIZED *point;
  if (! geo_parse(str, T_GEOMETRY, ',', &srid, &point))
    return NULL;
  if (! ensure_point_type(point) || ! ensure_not_empty(point) ||
      ! ensure_has_not_M_geo(point))
  {
    pfree(point);
    return NULL;
  }

  bool hasZ = FLAGS_GET_Z(point->gflags);

  if (! hasZ)
  {
    double theta;
    p_whitespace(str); p_comma(str); p_whitespace(str);
    if (! double_parse(str, &theta) || ! ensure_valid_rotation(theta))
      return NULL;
    POINT4D *p = (POINT4D *) GS_POINT_PTR(point);
    result = pose_make_2d(p->x, p->y, theta, srid);
  }
  else
  {
    double W, X, Y, Z;
    p_whitespace(str); p_comma(str); p_whitespace(str);
    if (! double_parse(str, &W)) 
      return NULL;
    p_whitespace(str); p_comma(str); p_whitespace(str);
    if (! double_parse(str, &X)) 
      return NULL;
    p_whitespace(str); p_comma(str); p_whitespace(str);
    if (! double_parse(str, &Y)) 
      return NULL;
    p_whitespace(str); p_comma(str); p_whitespace(str);
    if (! double_parse(str, &Z)) 
      return NULL;
    if (! ensure_unit_norm(W, X, Y, Z))
      return NULL;
    POINT4D *p = (POINT4D *) GS_POINT_PTR(point);
    result = pose_make_3d(p->x, p->y, p->z, W, X, Y, Z, srid);
  }
  pfree(point);

  /* Parse closing parenthesis */
  p_whitespace(str);
  if (! ensure_cparen(str, type_str) ||
        (end && ! ensure_end_input(str, type_str)))
    return NULL;

  return result;
}

#if MEOS
/**
 * @ingroup meos_pose_base_inout
 * @brief Return a pose from its string representation.
 * @param[in] str String
 * @csqlfn #Pose_in()
 */
Pose *
pose_in(const char *str)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(str, NULL);
  return pose_parse(&str, true);
}
#endif /* MEOS */

/**
 * @ingroup meos_pose_base_inout
 * @brief Return the string representation of a pose
 * @param[in] pose Pose
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Pose_out()
 */
char *
pose_out(const Pose *pose, int maxdd)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pose, NULL);
  if (! ensure_not_negative(maxdd))
    return NULL;

  char *result = palloc(MAXPOSELEN);
  GSERIALIZED *gs = pose_to_point(pose);
  char *point = basetype_out(PointerGetDatum(gs), T_GEOMETRY, maxdd);
  if (!MEOS_FLAGS_GET_Z(pose->flags))
  {
    char *theta = float8_out(pose->data[2], maxdd); /* theta if 2D*/
    snprintf(result, MAXPOSELEN - 1, "POSE (%s, %s)", point, theta);
    pfree(theta);
  }
  else
  {
    char *W = float8_out(pose->data[3], maxdd);
    char *X = float8_out(pose->data[4], maxdd);
    char *Y = float8_out(pose->data[5], maxdd);
    char *Z = float8_out(pose->data[6], maxdd);
    snprintf(result, MAXPOSELEN - 1, "POSE(%s, %s, %s, %s, %s)",
      point, W, X, Y, Z);
    pfree(W); pfree(X); pfree(Y); pfree(Z);
  }
  pfree(gs); pfree(point);
  return result;
}

/*****************************************************************************
 * Output in WKT and EWKT format
 *****************************************************************************/

/**
 * @brief Output a pose in the Well-Known Text (WKT) representation
 */
char *
pose_wkt_out(const Pose *pose, bool extended, int maxdd)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pose, NULL);
  if (! ensure_not_negative(maxdd))
    return NULL;

  /* Write the pose */
  bool hasz = MEOS_FLAGS_GET_Z(pose->flags);
  int32_t srid = pose_srid(pose);
  GSERIALIZED *gs = hasz ?
    geopoint_make(pose->data[0], pose->data[1], pose->data[2], true, false,
      srid) :
    geopoint_make(pose->data[0], pose->data[1], 0.0, false, false, srid);
  LWGEOM *geom = lwgeom_from_gserialized(gs);
  size_t len;
  char *wkt_point = lwgeom_to_wkt(geom, extended ? WKT_EXTENDED : WKT_ISO,
    maxdd, &len);
  /* Previous call added 1 (i.e., '\0') to len */
  len--;
  char *W, *X, *Y, *Z, *theta;
  if (hasz)
  {
    W = float8_out(pose->data[3], maxdd);
    X = float8_out(pose->data[4], maxdd);
    Y = float8_out(pose->data[5], maxdd);
    Z = float8_out(pose->data[6], maxdd);
    len += strlen(W) + strlen(X) + strlen(Y) + strlen(Z) + 3; // Three ','
  }
  else
  {
    theta = float8_out(pose->data[2], maxdd);
    len += strlen(theta) + 1; // One ','
  }
  len += 7; // Pose() + '\0' at the end
  char *result = palloc(len);
  if (hasz)
  {
    snprintf(result, len, "Pose(%s,%s,%s,%s,%s)", wkt_point, W, X, Y, Z);
    pfree(W); pfree(X); pfree(Y); pfree(Z); 
  }
  else
  {
    snprintf(result, len, "Pose(%s,%s)", wkt_point, theta);
    pfree(theta);
  }
  lwgeom_free(geom); pfree(wkt_point);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_pose_base_inout
 * @brief Return the Well-Known Text (WKT) representation of a pose
 * @param[in] pose Pose
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Pose_as_text()
 */
inline char *
pose_as_text(const Pose *pose, int maxdd)
{
  return pose_wkt_out(pose, false, maxdd);
}

/**
 * @ingroup meos_pose_base_inout
 * @brief Return the Extended Well-Known Text (EWKT) representation of a pose
 * @param[in] pose Pose
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Pose_as_ewkt()
 */
char *
pose_as_ewkt(const Pose *pose, int maxdd)
{
  VALIDATE_NOT_NULL(pose, NULL);
  return spatialbase_as_ewkt(PointerGetDatum(pose), T_POSE, maxdd);
}

/*****************************************************************************
 * WKB and HexWKB input/output functions for poses
 *****************************************************************************/

/**
 * @ingroup meos_pose_base_inout
 * @brief Return a pose from its Well-Known Binary (WKB) representation
 * @param[in] wkb WKB string
 * @param[in] size Size of the string
 * @csqlfn #Pose_recv(), #Pose_from_wkb()
 */
Pose *
pose_from_wkb(const uint8_t *wkb, size_t size)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(wkb, NULL);
  return DatumGetPoseP(type_from_wkb(wkb, size, T_POSE));
}

/**
 * @ingroup meos_pose_base_inout
 * @brief Return a pose from its ASCII hex-encoded Well-Known Binary (WKB)
 * representation
 * @param[in] hexwkb HexWKB string
 * @csqlfn #Pose_from_hexwkb()
 */
Pose *
pose_from_hexwkb(const char *hexwkb)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(hexwkb, NULL);
  size_t size = strlen(hexwkb);
  return DatumGetPoseP(type_from_hexwkb(hexwkb, size, T_POSE));
}

/*****************************************************************************/

/**
 * @ingroup meos_pose_base_inout
 * @brief Return the Well-Known Binary (WKB) representation of a pose
 * @param[in] pose Pose
 * @param[in] variant Output variant
 * @param[out] size_out Size of the output
 * @csqlfn #Pose_recv(), #Pose_as_wkb()
 */
uint8_t *
pose_as_wkb(const Pose *pose, uint8_t variant, size_t *size_out)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pose, NULL); VALIDATE_NOT_NULL(size_out, NULL);
  return datum_as_wkb(PointerGetDatum(pose), T_POSE, variant, size_out);
}

/**
 * @ingroup meos_pose_base_inout
 * @brief Return the ASCII hex-encoded Well-Known Binary (HexWKB)
 * representation of a pose
 * @param[in] pose Pose
 * @param[in] variant Output variant
 * @param[out] size_out Size of the output
 * @csqlfn #Pose_as_hexwkb()
 */
char *
pose_as_hexwkb(const Pose *pose, uint8_t variant, size_t *size_out)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pose, NULL); VALIDATE_NOT_NULL(size_out, NULL);
  return (char *) datum_as_wkb(PointerGetDatum(pose), T_POSE,
    variant | (uint8_t) WKB_HEX, size_out);
}

/*****************************************************************************
 * Constructors
 *****************************************************************************/

/**
 * @ingroup meos_pose_base_constructor
 * @brief Construct a 2D pose value from the arguments
 * @param[in] x,y Position
 * @param[in] theta Orientation
 * @param[in] srid SRID
 */
Pose *
pose_make_2d(double x, double y, double theta, int32_t srid)
{
  if (! ensure_valid_rotation(theta))
    return NULL;

  /* Ensure a unique representation for theta */
  if (theta == -M_PI)
    theta = M_PI;

  size_t memsize = DOUBLE_PAD(sizeof(Pose)) + 3 * sizeof(double);
  Pose *result = palloc0(memsize);
  SET_VARSIZE(result, memsize);
  MEOS_FLAGS_SET_X(result->flags, true);
  MEOS_FLAGS_SET_Z(result->flags, false);
  pose_set_srid(result, srid);
  result->data[0] = x;
  result->data[1] = y;
  result->data[2] = theta;
  return result;
}

/**
 * @ingroup meos_pose_base_constructor
 * @brief Construct a 2D pose value from a 2D point and a rotation angle
 * @param[in] gs 2D Point
 * @param[in] theta Orientation
 */
Pose *
pose_make_point2d(const GSERIALIZED *gs, double theta)
{
  /* Ensure validity of parameters */
  VALIDATE_NOT_NULL(gs, NULL);
  if (! ensure_valid_rotation(theta) || ! ensure_not_empty(gs) ||
      ! ensure_has_not_Z_geo(gs) || ! ensure_has_not_M_geo(gs))
    return NULL;

  /* Ensure a unique representation for theta */
  if (theta == -M_PI)
    theta = M_PI;

  POINT4D *p = (POINT4D *) GS_POINT_PTR(gs);
  const double *coordarr = (const double *) p;
  size_t memsize = DOUBLE_PAD(sizeof(Pose)) + 3 * sizeof(double);
  Pose *result = palloc0(memsize);
  SET_VARSIZE(result, memsize);
  MEOS_FLAGS_SET_X(result->flags, true);
  MEOS_FLAGS_SET_Z(result->flags, false);
  pose_set_srid(result, gserialized_get_srid(gs));
  result->data[0] = coordarr[0];
  result->data[1] = coordarr[1];
  result->data[2] = theta;
  return result;
}

/**
 * @ingroup meos_pose_base_constructor
 * @brief Construct a 3D pose value from the arguments
 * @param[in] x,y,z Position
 * @param[in] W,X,Y,Z Orientation
 * @param[in] srid SRID
 */
Pose *
pose_make_3d(double x, double y, double z, double W, double X, double Y,
  double Z, int32_t srid)
{
  if (! ensure_unit_norm(W, X, Y, Z))
      return NULL;

  /* Ensure a unique representation for the quaternion */
  if (W < 0.0)
  {
    W = -W;
    X = -X;
    Y = -Y;
    Z = -Z;
  }

  size_t memsize = DOUBLE_PAD(sizeof(Pose)) + 7 * sizeof(double);
  Pose *result = palloc0(memsize);
  SET_VARSIZE(result, memsize);
  MEOS_FLAGS_SET_X(result->flags, true);
  MEOS_FLAGS_SET_Z(result->flags, true);
  pose_set_srid(result, srid);
  result->data[0] = x;
  result->data[1] = y;
  result->data[2] = z;
  result->data[3] = W;
  result->data[4] = X;
  result->data[5] = Y;
  result->data[6] = Z;
  return result;
}

/**
 * @ingroup meos_pose_base_constructor
 * @brief Construct a 3D pose value from the arguments
 * @param[in] gs 3D Point
 * @param[in] W,X,Y,Z Orientation
 */
Pose *
pose_make_point3d(const GSERIALIZED *gs, double W, double X, double Y,
  double Z)
{
  /* Ensure validity of parameters */
  VALIDATE_NOT_NULL(gs, NULL);
  if (! ensure_unit_norm(W, X, Y, Z) || ! ensure_not_empty(gs) ||
      ! ensure_has_Z_geo(gs) || ! ensure_has_not_M_geo(gs))
    return NULL;

  /* Ensure a unique representation for the quaternion */
  if (W < 0.0)
  {
    W = -W;
    X = -X;
    Y = -Y;
    Z = -Z;
  }

  POINT4D *p = (POINT4D *) GS_POINT_PTR(gs);
  const double * coordarr = (const double *) p;
  size_t memsize = DOUBLE_PAD(sizeof(Pose)) + 7 * sizeof(double);
  Pose *result = palloc0(memsize);
  SET_VARSIZE(result, memsize);
  MEOS_FLAGS_SET_X(result->flags, true);
  MEOS_FLAGS_SET_Z(result->flags, true);
  pose_set_srid(result, gserialized_get_srid(gs));
  result->data[0] = coordarr[0];
  result->data[1] = coordarr[1];
  result->data[2] = coordarr[2];
  result->data[3] = W;
  result->data[4] = X;
  result->data[5] = Y;
  result->data[6] = Z;
  return result;
}

/**
 * @ingroup meos_pose_base_constructor
 * @brief Copy a pose value
 * @param[in] pose Pose
 */
Pose *
pose_copy(const Pose *pose)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pose, NULL);
  Pose *result = palloc(VARSIZE(pose));
  memcpy(result, pose, VARSIZE(pose));
  return result;
}

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

/**
 * @ingroup meos_pose_base_conversion
 * @brief Convert a pose into a geometry point
 * @param[in] pose Pose
 */
GSERIALIZED *
pose_to_point(const Pose *pose)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pose, NULL);

  LWPOINT *point;
  if (MEOS_FLAGS_GET_Z(pose->flags))
    point = lwpoint_make3dz(pose_srid(pose), pose->data[0], pose->data[1],
      pose->data[2]);
  else
    point = lwpoint_make2d(pose_srid(pose), pose->data[0], pose->data[1]);
  GSERIALIZED *gs = geo_serialize((LWGEOM *) point);
  lwpoint_free(point);
  return gs;
}

/**
 * @brief Convert a pose into a geometry point
 */
Datum
datum_pose_point(Datum pose)
{
  return GserializedPGetDatum(pose_to_point(DatumGetPoseP(pose)));
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_pose_conversion
 * @brief Return a geometry multipoint converted from an array of poses
 * @param[in] posearr Array of poses
 * @param[in] count Number of elements in the input array
 * @pre The argument @p count is greater than 1
 */
GSERIALIZED *
posearr_points(Pose **posearr, int count)
{
  VALIDATE_NOT_NULL(posearr, NULL); assert(count > 1);
  GSERIALIZED **geoms = palloc(sizeof(GSERIALIZED *) * count);
  /* SRID of the first element of the array */
  int32_t srid = pose_srid(posearr[0]);
  for (int i = 0; i < count; i++)
  {
    int32_t srid_elem = pose_srid(posearr[i]);
    if (! ensure_same_srid(srid, srid_elem))
    {
      for (int j = 0; j < i; j++)
        pfree(geoms[i]);
      pfree(geoms);
      return NULL;
    }
    geoms[i] = pose_to_point(posearr[i]);
  }
  GSERIALIZED *result = geo_collect_garray(geoms, count);
  pfree_array((void **) geoms, count);
  return result;
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup meos_pose_base_accessor
 * @brief Return the rotation of a 2D pose
 * @param[in] pose Pose
 * @return On error return @p DBL_MAX
 */
double
pose_rotation(const Pose *pose)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pose, DBL_MAX);
  if (! ensure_has_not_Z(T_POSE, pose->flags))
    return DBL_MAX;

  return pose->data[2];
}

/**
 * @brief Convert a pose into a geometry point
 */
Datum
datum_pose_rotation(Datum pose)
{
  return Float8GetDatum(pose_rotation(DatumGetPoseP(pose)));
}

/**
 * @ingroup meos_pose_base_accessor
 * @brief Return the orientation of a 3D pose
 * @param[in] pose Pose
 * @return On error return @p NULL
 */
double *
pose_orientation(const Pose *pose)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pose, NULL);
  if (! ensure_has_Z(T_POSE, pose->flags))
    return NULL;

  double *result = palloc(sizeof(double) * 4);
  result[0] = pose->data[3];
  result[1] = pose->data[4];
  result[2] = pose->data[5];
  result[3] = pose->data[6];
  return result;
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @ingroup meos_pose_base_transf
 * @brief Return a pose with the precision of the values set to a number of
 * decimal places
 * @param[in] pose Poses
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Pose_round()
 */
Pose *
pose_round(const Pose *pose, int maxdd)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pose, NULL);
  if (! ensure_positive(maxdd))
    return NULL;

  /* Set precision of the values */
  Pose *result;
  if (MEOS_FLAGS_GET_Z(pose->flags))
  {
    double x = float8_round(pose->data[0], maxdd);
    double y = float8_round(pose->data[1], maxdd);
    double z = float8_round(pose->data[2], maxdd);
    double W = float8_round(pose->data[3], maxdd);
    double X = float8_round(pose->data[4], maxdd);
    double Y = float8_round(pose->data[5], maxdd);
    double Z = float8_round(pose->data[6], maxdd);
    result = pose_make_3d(x, y, z, W, X, Y, Z, pose_srid(pose));
  }
  else
  {
    double x = float8_round(pose->data[0], maxdd);
    double y = float8_round(pose->data[1], maxdd);
    double theta = float8_round(pose->data[2], maxdd);
    result = pose_make_2d(x, y, theta, pose_srid(pose));
  }
  return result;
}

/**
 * @brief Return a pose with the precision of the values set to a number of
 * decimal places
 * @note Funcion used by the lifting infrastructure
 */
Datum
datum_pose_round(Datum pose, Datum size)
{
  /* Set precision of the values */
  return PointerGetDatum(pose_round(DatumGetPoseP(pose), DatumGetInt32(size)));
}

/**
 * @ingroup meos_pose_base_transf
 * @brief Return an array of poses with the precision of the vales set to a
 * number of decimal places
 * @param[in] posearr Array of poses
 * @param[in] count Number of elements in the array
 * @param[in] maxdd Maximum number of decimal digits
 * @csqlfn #Posearr_round()
 */
Pose **
posearr_round(const Pose **posearr, int count, int maxdd)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(posearr, NULL);
  if (! ensure_positive(count) || ! ensure_not_negative(maxdd))
    return NULL;

  Pose **result = palloc(sizeof(Pose *) * count);
  for (int i = 0; i < count; i++)
    result[i] = pose_round(posearr[i], maxdd);
  return result;
}

/*****************************************************************************
 * SRID functions
 *****************************************************************************/

/**
 * @ingroup meos_pose_base_srid
 * @brief Return the SRID
 * @param[in] pose Pose
 */
int32_t
pose_srid(const Pose *pose)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pose, SRID_INVALID);

  int32 srid = 0;
  srid = srid | (pose->srid[0] << 16);
  srid = srid | (pose->srid[1] << 8);
  srid = srid | (pose->srid[2]);
  /* Only the first 21 bits are set. Slide up and back to pull
     the negative bits down, if we need them. */
  srid = (srid<<11)>>11;

  /* 0 is our internal unknown value. We'll map back and forth here for now */
  if (srid == 0)
    return SRID_UNKNOWN;
  else
    return srid;
}

/**
 * @ingroup meos_pose_base_srid
 * @brief Set the SRID
 * @param[in] pose Pose
 * @param[in] srid SRID
 */
void
pose_set_srid(Pose *pose, int32_t srid)
{
  assert(pose);
  /* 0 is our internal unknown value.
   * We'll map back and forth here for now */
  if (srid == SRID_UNKNOWN)
    srid = 0;

  pose->srid[0] = (srid & 0x001F0000) >> 16;
  pose->srid[1] = (srid & 0x0000FF00) >> 8;
  pose->srid[2] = (srid & 0x000000FF);
}

/*****************************************************************************/

/**
 * @brief Return a pose transformed to another SRID using a pipeline
 * @param[in] pose Pose
 * @param[in] srid_to Target SRID, may be @p SRID_UNKNOWN for pipeline
 * transformation
 * @param[in] pj Information about the transformation
 */
Pose *
pose_transf_pj(const Pose *pose, int32_t srid_to, const LWPROJ *pj)
{
  VALIDATE_NOT_NULL(pose, NULL); VALIDATE_NOT_NULL(pj, NULL);
  /* Copy the pose to transform its point in place */
  Pose *result = pose_copy(pose);
  GSERIALIZED *gs = pose_to_point(pose);
  if (! point_transf_pj(gs, srid_to, pj))
  {
    pfree(result);
    return NULL;
  }
  POINT4D *p = (POINT4D *) GS_POINT_PTR(gs);
  /* Only the coordinates are transformed, not the orientation */
  const double * coordarr = (const double *) p;
  if (MEOS_FLAGS_GET_Z(pose->flags))
  {
    result->data[0] = coordarr[0];
    result->data[1] = coordarr[1];
    result->data[2] = coordarr[2];
  }
  else
  {
    result->data[0] = coordarr[0];
    result->data[1] = coordarr[1];
  }
  return result;
}

/**
 * @ingroup meos_pose_base_srid
 * @brief Return a pose transformed to another SRID
 * @param[in] pose Pose
 * @param[in] srid_to Target SRID
 */
Pose *
pose_transform(const Pose *pose, int32_t srid_to)
{
  int32_t srid_from;
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pose, NULL);
  if (! ensure_srid_known(srid_to) ||
      ! ensure_srid_known(srid_from = pose_srid(pose)))
    return NULL;
    
  /* Input and output SRIDs are equal, noop */
  if (srid_from == srid_to)
    return pose_copy(pose);

  /* Get the structure with information about the projection */
  LWPROJ *pj;
  if (! lwproj_lookup(srid_from, srid_to, &pj))
    return NULL;

  /* Transform the pose */
  return pose_transf_pj(pose, srid_to, pj);
}

/**
 * @ingroup meos_pose_base_srid
 * @brief Return a pose transformed to another SRID using a
 * pipeline
 * @param[in] pose Pose
 * @param[in] pipeline Pipeline string
 * @param[in] srid_to Target SRID, may be @p SRID_UNKNOWN
 * @param[in] is_forward True when the transformation is forward
 */
Pose *
pose_transform_pipeline(const Pose *pose, const char *pipeline,
  int32_t srid_to, bool is_forward)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pose, NULL); VALIDATE_NOT_NULL(pipeline, NULL);
  if (! ensure_srid_known(srid_to))
    return NULL;

  /* There is NO test verifying whether the input and output SRIDs are equal */

  /* Get the structure with information about the projection */
  LWPROJ *pj = lwproj_from_str_pipeline(pipeline, is_forward);
  if (! pj)
    return NULL;

  /* Transform the pose */
  Pose *result = pose_transf_pj(pose, srid_to, pj);

  /* Transform the pose */
  proj_destroy(pj->pj); pfree(pj);
  return result;
}

/*****************************************************************************
 * Distance function
 *****************************************************************************/

/**
 * @brief Return the distance between the two poses
 */
Datum
pose_distance(Datum pose1, Datum pose2)
{
  Datum geom1 = PosePGetDatum(pose_to_point(DatumGetPoseP(pose1)));
  Datum geom2 = PosePGetDatum(pose_to_point(DatumGetPoseP(pose2)));
  return datum_pt_distance2d(geom1, geom2);
}

/**
 * @ingroup meos_pose_base_dist
 * @brief Return the distance between two poses
 * @return On error return -1.0
 * @csqlfn Distance_pose_pose()
 */
double
distance_pose_pose(const Pose *pose1, const Pose *pose2)
{
  VALIDATE_NOT_NULL(pose1, -1.0); VALIDATE_NOT_NULL(pose2, -1.0);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_pose_pose(pose1, pose2))
    return -1.0;
  /* The following function assumes that all validity tests have been done */
  return pose_distance(PointerGetDatum(pose1), PointerGetDatum(pose2));
}

/**
 * @ingroup meos_internal_pose_dist
 * @brief Return the distance between two circular buffers
 * @param[in] pose1,pose2 Poses
 * @note The function assumes that all validity tests have been previously done
 */
Datum
datum_pose_distance(Datum pose1, Datum pose2)
{
  return Float8GetDatum(pose_distance(pose1, pose2));
}

/*****************************************************************************/

/**
 * @ingroup meos_pose_base_dist
 * @brief Return the distance between a pose and a geometry
 * @return On error return -1.0
 * @csqlfn Distance_pose_geo()
 */
double
distance_pose_geo(const Pose *pose, const GSERIALIZED *gs)
{
  VALIDATE_NOT_NULL(pose, -1.0); VALIDATE_NOT_NULL(gs, -1.0);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_pose_geo(pose, gs) || gserialized_is_empty(gs))
    return -1.0;

  GSERIALIZED *geo = pose_to_point(pose);
  double result = geom_distance2d(geo, gs);
  pfree(geo);
  return result;
}

/**
 * @ingroup meos_pose_base_dist
 * @brief Return the distance between a pose and a spatiotemporal box
 * @return On error return -1.0
 * @csqlfn Distance_pose_stbox()
 */
double
distance_pose_stbox(const Pose *pose, const STBox *box)
{
  VALIDATE_NOT_NULL(pose, -1.0); VALIDATE_NOT_NULL(box, -1.0);
  /* Ensure the validity of the arguments */
  if (! ensure_valid_pose_stbox(pose, box))
    return -1.0;

  GSERIALIZED *geo1 = pose_to_point(pose);
  GSERIALIZED *geo2 = stbox_geo(box);
  double result = geom_distance2d(geo1, geo2);
  pfree(geo1); pfree(geo2); 
  return result;
}

/*****************************************************************************
 * Comparison functions for defining B-tree indexes
 *****************************************************************************/

/**
 * @ingroup meos_pose_base_comp
 * @brief Return true if the first pose is equal to the second one
 * @param[in] pose1,pose2 Poses
 */
bool
pose_eq(const Pose *pose1, const Pose *pose2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pose1, NULL); VALIDATE_NOT_NULL(pose2, NULL);

  if (MEOS_FLAGS_GET_Z(pose1->flags) != MEOS_FLAGS_GET_Z(pose2->flags) ||
      pose_srid(pose1) != pose_srid(pose2))
    return false;
  bool result = (
    float8_eq(pose1->data[0], pose2->data[0]) &&
    float8_eq(pose1->data[1], pose2->data[1]) &&
    float8_eq(pose1->data[2], pose2->data[2])
  );
  if (MEOS_FLAGS_GET_Z(pose1->flags))
    result &= (
      float8_eq(pose1->data[3], pose2->data[3]) &&
      float8_eq(pose1->data[4], pose2->data[4]) &&
      float8_eq(pose1->data[5], pose2->data[5]) &&
      float8_eq(pose1->data[6], pose2->data[6])
    );
  return result;
}

/**
 * @ingroup meos_pose_base_comp
 * @brief Return true if the first pose is not equal to the second one
 * @param[in] pose1,pose2 Poses
 */
inline bool
pose_ne(const Pose *pose1, const Pose *pose2)
{
  return ! pose_eq(pose1, pose2);
}

/**
 * @ingroup meos_pose_base_comp
 * @brief Return true if the first pose is equal to the second one
 * @param[in] pose1,pose2 Poses
 */
bool
pose_same(const Pose *pose1, const Pose *pose2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pose1, NULL); VALIDATE_NOT_NULL(pose2, NULL);

  if (MEOS_FLAGS_GET_Z(pose1->flags) != MEOS_FLAGS_GET_Z(pose2->flags) ||
      pose_srid(pose1) != pose_srid(pose2))
    return false;
  bool result = (
    MEOS_FP_EQ(pose1->data[0], pose2->data[0]) &&
    MEOS_FP_EQ(pose1->data[1], pose2->data[1]) &&
    MEOS_FP_EQ(pose1->data[2], pose2->data[2])
  );
  if (MEOS_FLAGS_GET_Z(pose1->flags))
    result &= (
      MEOS_FP_EQ(pose1->data[3], pose2->data[3]) &&
      MEOS_FP_EQ(pose1->data[4], pose2->data[4]) &&
      MEOS_FP_EQ(pose1->data[5], pose2->data[5]) &&
      MEOS_FP_EQ(pose1->data[6], pose2->data[6])
    );
  return result;
}

/**
 * @ingroup meos_pose_base_comp
 * @brief Return true if the first pose is not equal to the second one
 * @param[in] pose1,pose2 Poses
 */
inline bool
pose_nsame(const Pose *pose1, const Pose *pose2)
{
  return ! pose_same(pose1, pose2);
}

/**
 * @ingroup meos_pose_base_comp
 * @brief Return -1, 0, or 1 depending on whether the first pose
 * is less than, equal to, or greater than the second one
 * @param[in] pose1,pose2 Poses
 */
int
pose_cmp(const Pose *pose1, const Pose *pose2)
{
  /* Ensure the validity of the arguments */
  assert(pose1); assert(pose2);

  /* Compare first the dimension, then the SRID,
     then the position, then the orientation */
  bool hasz1 = MEOS_FLAGS_GET_Z(pose1->flags),
       hasz2 = MEOS_FLAGS_GET_Z(pose2->flags);
  if (hasz1 != hasz2)
    return (hasz1 ? 1 : -1);

  int32 srid1 = pose_srid(pose1),
        srid2 = pose_srid(pose2);
  if (srid1 < srid2)
    return -1;
  else if (srid1 > srid2)
    return 1;

  if (hasz1)
    return memcmp(pose1->data, pose2->data, sizeof(double) * 7);
  else
    return memcmp(pose1->data, pose2->data, sizeof(double) * 3);
}

/**
 * @ingroup meos_pose_base_comp
 * @brief Return true if the first pose is less than the second one
 * @param[in] pose1,pose2 Poses
 */
inline bool
pose_lt(const Pose *pose1, const Pose *pose2)
{
  return pose_cmp(pose1, pose2) < 0;
}

/**
 * @ingroup meos_pose_base_comp
 * @brief Return true if the first pose is less than or equal to the second one
 * @param[in] pose1,pose2 Poses
 */
inline bool
pose_le(const Pose *pose1, const Pose *pose2)
{
  return pose_cmp(pose1, pose2) <= 0;
}

/**
 * @ingroup meos_pose_base_comp
 * @brief Return true if the first pose is greater than the second one
 * @param[in] pose1,pose2 Poses
 */
inline bool
pose_gt(const Pose *pose1, const Pose *pose2)
{
  return pose_cmp(pose1, pose2) > 0;
}

/**
 * @ingroup meos_pose_base_comp
 * @brief Return true if the first pose is greater than or equal to the second
 * one
 * @param[in] pose1,pose2 Poses
 */
inline bool
pose_ge(const Pose *pose1, const Pose *pose2)
{
  return pose_cmp(pose1, pose2) >= 0;
}

/*****************************************************************************
 * Function for defining hash indexes
 * The function reuses the approach for span types for combining the hash of
 * the lower and upper bounds.
 *****************************************************************************/

/* Prototype for liblwgeom/lookup3.c */
/* key = the key to hash */
/* length = length of the key */
/* pc = IN: primary initval, OUT: primary hash */
/* pb = IN: secondary initval, OUT: secondary hash */
void hashlittle2(const void *key, size_t length, uint32_t *pc, uint32_t *pb);

/**
 * @ingroup meos_pose_base_accessor
 * @brief Return the 32-bit hash value of a pose
 * @param[in] pose Pose
 */
uint32
pose_hash(const Pose *pose)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(pose, INT_MAX);

  /* Use same code as gserialized2_hash */
  int32_t hval;
  int32_t pb = 0, pc = 0;
  /* Point to just the type/coordinate part of buffer */
  size_t hsz1 = 8; /* varsize (4) + flags (1) + srid(3) */
  uint8_t *b1 = (uint8_t *) pose + hsz1;
  /* Calculate size of type/coordinate buffer */
  size_t sz1 = VARSIZE(pose);
  size_t bsz1 = sz1 - hsz1;
  /* Calculate size of srid/type/coordinate buffer */
  int32_t srid = pose_srid(pose);
  size_t bsz2 = bsz1 + sizeof(int);
  uint8_t *b2 = palloc(bsz2);
  /* Copy srid into front of combined buffer */
  memcpy(b2, &srid, sizeof(int));
  /* Copy type/coordinates into rest of combined buffer */
  memcpy(b2 + sizeof(int), b1, bsz1);
  /* Hash combined buffer */
  hashlittle2(b2, bsz2, (uint32_t *) &pb, (uint32_t *) &pc);
  pfree(b2);
  hval = pb ^ pc;
  return hval;
}

/**
 * @ingroup meos_pose_base_accessor
 * @brief Return the 64-bit hash value of a pose using a seed
 * @param[in] pose Pose
 * @param[in] seed Seed
 * csqlfn hash_extended
 */
uint64
pose_hash_extended(const Pose *pose, uint64 seed)
{
  /* PostGIS currently does not provide an extended hash function, */
  return DatumGetUInt64(hash_any_extended(
    (unsigned char *) VARDATA_ANY(pose), VARSIZE_ANY_EXHDR(pose), seed));
}

/*****************************************************************************/
