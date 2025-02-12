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

#ifndef __POSE_H__
#define __POSE_H__

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>

/*****************************************************************************
 * Struct definitions
 *****************************************************************************/

/**
 * Structure to represent a pose values
 *
 * flags (8 bits, x = unused): xxZXxxxx
 * data: 2D: [x, y, theta]
 *       3D: [x, y, z, W, X, Z, Y]
 *
 */
typedef struct
{
  int32         vl_len_;       /**< varlena header (do not touch directly!) */
  int8          flags;         /**< flags */
  uint8_t       srid[3];       /**< srid */
  double        data[];        /**< position and orientation values */
} Pose;

/*****************************************************************************
 * fmgr macros
 *****************************************************************************/

#define DatumGetPoseP(X)         ((Pose *) DatumGetPointer(X))
#define PosePGetDatum(X)         PointerGetDatum(X)
#define PG_GETARG_POSE_P(X)      DatumGetPoseP(PG_GETARG_DATUM(X))
#define PG_RETURN_POSE_P(X)      PG_RETURN_POINTER(X)

/*****************************************************************************
 * tpose_static.c
 *****************************************************************************/

extern Pose *pose_in(const char *str, bool end);
extern char *pose_out(const Pose *pose, int maxdd);

extern Pose *pose_make_2d(double x, double y, double theta);
extern Pose *pose_make_3d(double x, double y, double z,
  double W, double X, double Y, double Z);
extern Pose *pose_copy(Pose *pose);

extern int32_t pose_srid(const Pose *pose);
extern void pose_set_srid(Pose *pose, int32_t srid);

extern GSERIALIZED *pose_geom(const Pose *pose);
extern Datum datum_pose_geom(Datum pose);

/* Distance */

extern Datum pose_distance(Datum pose1, Datum pose2);

/* Interpolation */

extern Pose *pose_interpolate(const Pose *pose1, const Pose *pose2, double ratio);
extern bool pose_collinear(const Pose *pose1, const Pose *pose2, const Pose *pose3, double ratio);

/* Comparison functions */

extern bool pose_eq(const Pose *pose1, const Pose *pose2);
extern bool pose_ne(const Pose *pose1, const Pose *pose2);

extern bool pose_same(const Pose *pose1, const Pose *pose2);
extern bool pose_nsame(const Pose *pose1, const Pose *pose2);

extern int pose_cmp(const Pose *pose1, const Pose *pose2);
extern bool pose_lt(const Pose *pose1, const Pose *pose2);
extern bool pose_le(const Pose *pose1, const Pose *pose2);
extern bool pose_gt(const Pose *pose1, const Pose *pose2);
extern bool pose_ge(const Pose *pose1, const Pose *pose2);

/* Hash functions */

extern uint32 pose_hash(const Pose *pose);
extern uint64 pose_hash_extended(const Pose *pose, uint64 seed);

/*****************************************************************************/

#endif /* __POSE_H__ */
