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
 * @brief Basic functions for static pose objects.
 */


#include "pose/tpose_static.h"

/* C */
#include <math.h>
/* Postgres */
#include <postgres.h>
#if POSTGRESQL_VERSION_NUMBER >= 160000
  #include "varatt.h"
#endif
/* MobilityDB */
#include "meos.h"
#include "meos_internal.h"
#include "general/pg_types.h"
#include "point/tpoint_spatialfuncs.h"
#include "pose/tpose_parser.h"

/** Buffer size for input and output of pose values */
#define MAXPOSELEN    128

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/**
 * @brief Return a network point from its string representation.
 */
Pose *
pose_in(const char *str, bool end)
{
  return pose_parse(&str, end);
}

/**
 * @brief Return the string representation of a network point
 */
char *
pose_out(const Pose *pose, int maxdd)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) pose) || ! ensure_not_negative(maxdd))
    return NULL;

  char *result = palloc(MAXPOSELEN);
  char *x = float8_out(pose->data[0], maxdd);
  char *y = float8_out(pose->data[1], maxdd);
  char *z = float8_out(pose->data[2], maxdd); /* theta if 2D*/
  if (!MEOS_FLAGS_GET_Z(pose->flags))
  {
    snprintf(result, MAXPOSELEN - 1, "POSE (%s, %s, %s)", x, y, z);
  }
  else
  {
    char *W = float8_out(pose->data[3], maxdd);
    char *X = float8_out(pose->data[4], maxdd);
    char *Y = float8_out(pose->data[5], maxdd);
    char *Z = float8_out(pose->data[6], maxdd);
    snprintf(result, MAXPOSELEN - 1, "POSE Z (%s, %s, %s, %s, %s, %s, %s)",
      x, y, z, W, X, Y, Z);
    pfree(W); pfree(X); pfree(Y); pfree(Z);
  }
  pfree(x); pfree(y); pfree(z);
  return result;
}

/*****************************************************************************
 * Constructors
 *****************************************************************************/

/**
 * Construct a 2d pose value from the arguments
 */
Pose *
pose_make_2d(double x, double y, double theta)
{
  if (theta < -M_PI || theta > M_PI)
    meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE,
      "Rotation angle must be in ]-pi, pi]. Recieved: %f", theta);

  /* If we want a unique representation for theta */
  if (theta == -M_PI)
    theta = M_PI;

  size_t memsize = DOUBLE_PAD(sizeof(Pose)) + 3 * sizeof(double);
  Pose *result = palloc0(memsize);
  SET_VARSIZE(result, memsize);
  MEOS_FLAGS_SET_X(result->flags, true);
  MEOS_FLAGS_SET_Z(result->flags, false);
  result->data[0] = x;
  result->data[1] = y;
  result->data[2] = theta;
  return result;
}

/**
 * Construct a 3d pose value from the arguments
 */
Pose *
pose_make_3d(double x, double y, double z,
  double W, double X, double Y, double Z)
{
  if (fabs(sqrt(W*W + X*X + Y*Y + Z*Z) - 1)  > MEOS_EPSILON)
    meos_error(ERROR, MEOS_ERR_VALUE_OUT_OF_RANGE,
      "Rotation quaternion must be of unit norm. Recieved: %f",
      sqrt(W*W + X*X + Y*Y + Z*Z));

  /* If we want a unique representation for the quaternion */
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
  result->data[0] = x;
  result->data[1] = y;
  result->data[2] = z;
  result->data[3] = W;
  result->data[4] = X;
  result->data[5] = Y;
  result->data[6] = Z;
  return result;
}

Pose *
pose_copy(Pose *pose)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) pose))
    return NULL;
  Pose *result = palloc(VARSIZE(pose));
  memcpy(result, pose, VARSIZE(pose));
  return result;
}

/*****************************************************************************
 * SRID functions
 *****************************************************************************/

int32
pose_get_srid(const Pose *pose)
{
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

void
pose_set_srid(Pose *pose, int32 srid)
{
  srid = clamp_srid(srid);

  /* 0 is our internal unknown value.
   * We'll map back and forth here for now */
  if (srid == SRID_UNKNOWN)
    srid = 0;

  pose->srid[0] = (srid & 0x001F0000) >> 16;
  pose->srid[1] = (srid & 0x0000FF00) >> 8;
  pose->srid[2] = (srid & 0x000000FF);
}

/*****************************************************************************
 * Cast functions
 *****************************************************************************/

/**
 * @brief Transforms the pose into a geometry point
 */
GSERIALIZED *
pose_geom(const Pose *pose)
{
  LWPOINT *point;
  if (MEOS_FLAGS_GET_Z(pose->flags))
    point = lwpoint_make3dz(pose_get_srid(pose),
      pose->data[0], pose->data[1], pose->data[2]);
  else
    point = lwpoint_make2d(pose_get_srid(pose),
      pose->data[0], pose->data[1]);
  GSERIALIZED *gs = geo_serialize((LWGEOM *)point);
  lwpoint_free(point);
  return gs;
}

/**
 * @brief Transforms the pose into a geometry point
 */
Datum
datum_pose_geom(Datum pose)
{
  return PosePGetDatum(pose_geom(DatumGetPoseP(pose)));
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
  Datum geom1 = PosePGetDatum(pose_geom(DatumGetPoseP(pose1)));
  Datum geom2 = PosePGetDatum(pose_geom(DatumGetPoseP(pose2)));
  return pt_distance2d(geom1, geom2);
}

/*****************************************************************************
 * Functions for defining B-tree index
 *****************************************************************************/

/**
 * Returns true if the first pose is equal to the second one
 */
bool
pose_eq(const Pose *pose1, const Pose *pose2)
{
  if (MEOS_FLAGS_GET_Z(pose1->flags) != MEOS_FLAGS_GET_Z(pose2->flags)
    || pose_get_srid(pose1) != pose_get_srid(pose2))
    return false;
  bool result = (fabs(pose1->data[0] - pose2->data[0]) < MEOS_EPSILON &&
    fabs(pose1->data[1] - pose2->data[1]) < MEOS_EPSILON &&
    fabs(pose1->data[2] - pose2->data[2]) < MEOS_EPSILON);
  if (MEOS_FLAGS_GET_Z(pose1->flags))
    result &= (fabs(pose1->data[3] - pose2->data[3]) < MEOS_EPSILON &&
    fabs(pose1->data[4] - pose2->data[4]) < MEOS_EPSILON &&
    fabs(pose1->data[5] - pose2->data[5]) < MEOS_EPSILON &&
    fabs(pose1->data[6] - pose2->data[6]) < MEOS_EPSILON);
  return result;
}

/*****************************************************************************/

Pose *
pose_interpolate(const Pose *pose1, const Pose *pose2, double ratio)
{
  Pose *result;
  if (!MEOS_FLAGS_GET_Z(pose1->flags))
  {
    double x = pose1->data[0] * (1 - ratio) + pose2->data[0] * ratio;
    double y = pose1->data[1] * (1 - ratio) + pose2->data[1] * ratio;
    double theta;
    double theta_delta = pose2->data[2] - pose1->data[2];
    /* If fabs(theta_delta) == M_PI: Always turn counter-clockwise */
    if (fabs(theta_delta) < MEOS_EPSILON)
        theta = pose1->data[2];
    else if (theta_delta > 0 && fabs(theta_delta) <= M_PI)
        theta = pose1->data[2] + theta_delta*ratio;
    else if (theta_delta > 0 && fabs(theta_delta) > M_PI)
        theta = pose2->data[2] + (2*M_PI - theta_delta)*(1 - ratio);
    else if (theta_delta < 0 && fabs(theta_delta) < M_PI)
        theta = pose1->data[2] + theta_delta*ratio;
    else /* (theta_delta < 0 && fabs(theta_delta) >= M_PI) */
        theta = pose1->data[2] + (2*M_PI + theta_delta)*ratio;
    if (theta > M_PI)
        theta = theta - 2*M_PI;
    result = pose_make_2d(x, y, theta);
  }
  else
  {
    double x = pose1->data[0] * (1 - ratio) + pose2->data[0] * ratio;
    double y = pose1->data[1] * (1 - ratio) + pose2->data[1] * ratio;
    double z = pose1->data[2] * (1 - ratio) + pose2->data[2] * ratio;
    double W, W1 = pose1->data[3], W2 = pose2->data[3];
    double X, X1 = pose1->data[4], X2 = pose2->data[4];
    double Y, Y1 = pose1->data[5], Y2 = pose2->data[5];
    double Z, Z1 = pose1->data[6], Z2 = pose2->data[6];
    double dot =  W1*W2 + X1*X2 + Y1*Y2 + Z1*Z2;
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
      W = W1 + (W2 - W1)*ratio;
      X = X1 + (X2 - X1)*ratio;
      Y = Y1 + (Y2 - Y1)*ratio;
      Z = Z1 + (Z2 - Z1)*ratio;
    }
    else
    {
      double theta_0 = acos(dot);
      double theta = theta_0*ratio;
      double sin_theta = sin(theta);
      double sin_theta_0 = sin(theta_0);
      double s1 = cos(theta) - dot * sin_theta / sin_theta_0;
      double s2 = sin_theta / sin_theta_0;
      W = W1*s1 + W2*s2;
      X = X1*s1 + X2*s2;
      Y = Y1*s1 + Y2*s2;
      Z = Z1*s1 + Z2*s2;
    }
    double norm = W*W + X*X + Y*Y + Z*Z;
    W /= norm;
    X /= norm;
    Y /= norm;
    Z /= norm;
    result = pose_make_3d(x, y, z, W, X, Y, Z);
  }
  return result;
}

/*****************************************************************************/
