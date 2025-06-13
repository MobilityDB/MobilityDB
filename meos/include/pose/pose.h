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

#ifndef __POSE_H__
#define __POSE_H__

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_pose.h>

/*****************************************************************************
 * Struct definitions
 *****************************************************************************/

/**
 * @brief Structure to represent pose values
 * @details
 * flags (8 bits, x = unused): xxZXxxxx
 * data: 2D: [x, y, theta]
 *       3D: [x, y, z, W, X, Z, Y]
 *
 */
struct Pose
{
  int32         vl_len_;       /**< varlena header (do not touch directly!) */
  int8          flags;         /**< flags */
  uint8_t       srid[3];       /**< srid */
  double        data[];        /**< position and orientation values */
};

/*****************************************************************************
 * fmgr macros
 *****************************************************************************/

#define DatumGetPoseP(X)         ((Pose *) DatumGetPointer(X))
#define PosePGetDatum(X)         PointerGetDatum(X)
#define PG_GETARG_POSE_P(X)      DatumGetPoseP(PG_GETARG_DATUM(X))
#define PG_RETURN_POSE_P(X)      PG_RETURN_POINTER(X)

/*****************************************************************************/

/* Validity functions */

extern bool ensure_valid_pose_geo(const Pose *pose, const GSERIALIZED *gs);
extern bool ensure_valid_pose_stbox(const Pose *pose, const STBox *box);
extern bool ensure_valid_pose_pose(const Pose *pose1, const Pose *pose2);
extern bool ensure_valid_poseset_pose(const Set *s, const Pose *pose);

/* Collinear and interpolation functions */

extern bool pose_collinear(const Pose *pose1, const Pose *pose2,
  const Pose *pose3, double ratio);
extern Pose *posesegm_interpolate(const Pose *start, const Pose *end,
  double ratio);
extern long double posesegm_locate(const Pose *start, const Pose *end,
  const Pose *value);

/* Input/output functions */

extern char *pose_wkt_out(const Pose *pose, bool extended, int maxdd);

extern Pose *pose_parse(const char **str, bool end);

extern Datum datum_pose_point(Datum pose);
extern Datum datum_pose_rotation(Datum pose);

/* Transformation functions */

extern Datum datum_pose_round(Datum pose, Datum size);

/* Distance */

extern Datum pose_distance(Datum pose1, Datum pose2);

/* Box functions */

extern bool pose_set_stbox(const Pose *pose, STBox *box);
extern void posearr_set_stbox(const Datum *values, int count, STBox *box);
extern bool pose_timestamptz_set_stbox(const Pose *pose, TimestampTz t,
  STBox *box);
extern bool pose_tstzspan_set_stbox(const Pose *pose, const Span *p,
  STBox *box);

/*****************************************************************************/

#endif /* __POSE_H__ */
