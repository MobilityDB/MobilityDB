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
 * @brief External API of the Mobility Engine Open Source (MEOS) library
 */

#ifndef __MEOS_POSE_H__
#define __MEOS_POSE_H__

/* C */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include <meos_internal.h>
#include "geo/stbox.h"

/*****************************************************************************
 * Struct definitions
 *****************************************************************************/

/**
 * Opaque structure to represent pose values
 */
typedef struct Pose Pose;

/*****************************************************************************
 * fmgr macros
 *****************************************************************************/

#define DatumGetPoseP(X)         ((Pose *) DatumGetPointer(X))
#define PosePGetDatum(X)         PointerGetDatum(X)
#define PG_GETARG_POSE_P(X)      DatumGetPoseP(PG_GETARG_DATUM(X))
#define PG_RETURN_POSE_P(X)      PG_RETURN_POINTER(X)

/*****************************************************************************
 * Validity macros
 *****************************************************************************/

/**
 * @brief Macro for ensuring that the set passed as argument is a pose set
 */
#if MEOS
  #define VALIDATE_POSESET(set, ret) \
    do { \
          if (! ensure_not_null((void *) set) || \
              ! ensure_set_isof_type((set), T_POSESET) ) \
           return (ret); \
    } while (0)
#else
  #define VALIDATE_POSESET(set, ret) \
    do { \
      assert(set); \
      assert(set_isof_type((set), T_POSESET)); \
    } while (0)
#endif

/**
 * @brief Macro for ensuring that the temporal value passed as argument is a
 * temporal pose
 * @note The macro works for the Temporal type and its subtypes TInstant,
 * TSequence, and TSequenceSet
 */
#if MEOS
  #define VALIDATE_TPOSE(temp, ret) \
    do { \
          if (! ensure_not_null((void *) (temp)) || \
              ! ensure_temporal_isof_type((Temporal *) (temp), T_TPOSE) ) \
           return (ret); \
    } while (0)
#else
  #define VALIDATE_TPOSE(temp, ret) \
    do { \
      assert(temp); \
      assert(((Temporal *) (temp))->temptype == T_TPOSE); \
    } while (0)
#endif

/******************************************************************************
 * Functions for poses
 ******************************************************************************/

extern char *pose_as_text(const Pose *pose, int maxdd);
extern char *pose_as_ewkt(const Pose *pose, int maxdd);
extern uint8_t *pose_as_wkb(const Pose *pose, uint8_t variant, size_t *size_out);
extern char *pose_as_hexwkb(const Pose *pose, uint8_t variant, size_t *size);
extern int pose_cmp(const Pose *pose1, const Pose *pose2);
extern Pose *pose_copy(const Pose *pose);
extern bool pose_eq(const Pose *pose1, const Pose *pose2);
extern Pose *pose_from_wkb(const uint8_t *wkb, size_t size);
extern Pose *pose_from_hexwkb(const char *hexwkb);
extern bool pose_ge(const Pose *pose1, const Pose *pose2);
extern bool pose_gt(const Pose *pose1, const Pose *pose2);
extern uint32 pose_hash(const Pose *pose);
extern uint64 pose_hash_extended(const Pose *pose, uint64 seed);
extern Pose *pose_in(const char *str);
extern bool pose_le(const Pose *pose1, const Pose *pose2);
extern bool pose_lt(const Pose *pose1, const Pose *pose2);
extern Pose *pose_make_2d(double x, double y, double theta, int32_t srid);
extern Pose *pose_make_3d(double x, double y, double z, double W, double X, double Y, double Z, int32_t srid);
extern Pose *pose_make_point2d(const GSERIALIZED *gs, double theta);
extern Pose *pose_make_point3d(const GSERIALIZED *gs, double W, double X, double Y, double Z);
extern bool pose_ne(const Pose *pose1, const Pose *pose2);
extern bool pose_nsame(const Pose *pose1, const Pose *pose2);
extern char *pose_out(const Pose *pose, int maxdd);
extern GSERIALIZED *pose_point(const Pose *pose);
extern double pose_rotation(const Pose *pose);
extern double *pose_orientation(const Pose *pose);
extern Pose *pose_round(const Pose *pose, int maxdd);
extern bool pose_same(const Pose *pose1, const Pose *pose2);
extern int32_t pose_srid(const Pose *pose);
extern void pose_set_srid(Pose *pose, int32_t srid);
extern Pose *pose_transform(const Pose *pose, int32 srid);
extern Pose *pose_transform_pipeline(const Pose *pose, const char *pipelinestr, int32 srid, bool is_forward);
extern Pose *geom_pose(const GSERIALIZED *gs);

/******************************************************************************
 * Functions for pose sets
 ******************************************************************************/

extern bool contained_pose_set(const Pose *pose, const Set *s);
extern bool contains_set_pose(const Set *s, Pose *pose);
extern Set *intersection_pose_set(const Pose *pose, const Set *s);
extern Set *intersection_set_pose(const Set *s, const Pose *pose);
extern Set *minus_pose_set(const Pose *pose, const Set *s);
extern Set *minus_set_pose(const Set *s, const Pose *pose);
extern Pose *poseset_end_value(const Set *s);
extern Set *poseset_in(const char *str);
extern Set *poseset_make(const Pose **values, int count);
extern char *poseset_out(const Set *s, int maxdd);
extern Set *pose_to_set(const Pose *pose);
extern Pose *poseset_start_value(const Set *s);
extern Set *pose_union_transfn(Set *state, const Pose *pose);
extern bool poseset_value_n(const Set *s, int n, Pose **result);
extern Pose **poseset_values(const Set *s);
extern Set *union_pose_set(const Pose *pose, const Set *s);
extern Set *union_set_pose(const Set *s, const Pose *pose);

/*===========================================================================*
 * Functions for box types
 *===========================================================================*/

/*****************************************************************************
 * Constructor functions for box types
 *****************************************************************************/

extern STBox *pose_tstzspan_to_stbox(const Pose *pose, const Span *s);
extern STBox *pose_timestamptz_to_stbox(const Pose *pose, TimestampTz t);

/*****************************************************************************
 * Conversion functions for box types
 *****************************************************************************/

extern STBox *pose_stbox(const Pose *pose);

/*===========================================================================*
 * Functions for temporal poses
 *===========================================================================*/

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

extern char *tpose_as_ewkt(const Temporal *temp, int maxdd);
extern char *tpose_as_text(const Temporal *temp, int maxdd);
extern char *tpose_out(const Temporal *temp, int maxdd);

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

extern Temporal *tpoint_tfloat_to_tpose(const Temporal *tpoint, const Temporal *tradius);

/*****************************************************************************
 * Restriction functions
 *****************************************************************************/

// extern Temporal *tpose_at_geom(const Temporal *temp, const GSERIALIZED *gs, const Span *zspan);
// extern Temporal *tpose_at_pose(const Temporal *temp, const Pose *pose, const Span *zspan);
// extern Temporal *tpose_at_stbox(const Temporal *temp, const STBox *box, bool border_inc);
// extern Temporal *tpose_minus_geom(const Temporal *temp, const GSERIALIZED *gs, const Span *zspan);
// extern Temporal *tpose_minus_pose(const Temporal *temp, const Pose *pose, const Span *zspan);
// extern Temporal *tpose_minus_stbox(const Temporal *temp, const STBox *box, bool border_inc);

/*****************************************************************************
 * Distance functions
 *****************************************************************************/

extern double distance_pose_geo(const Pose *pose, const GSERIALIZED *gs);
extern double distance_pose_stbox(const Pose *pose, const STBox *box);
extern double distance_pose_pose(const Pose *pose1, const Pose *pose2);
extern Temporal *distance_tpose_pose(const Temporal *temp, const Pose *pose);
extern Temporal *distance_tpose_point(const Temporal *temp, const GSERIALIZED *gs);
extern Temporal *distance_tpose_tpose(const Temporal *temp1, const Temporal *temp2);
extern double nad_stbox_tpose(const STBox *box, const Temporal *temp);
extern double nad_tpose_geo(const Temporal *temp, const GSERIALIZED *gs);
extern double nad_tpose_pose(const Temporal *temp, const Pose *pose);
extern double nad_tpose_stbox(const Temporal *temp, const STBox *box);
extern double nad_tpose_tpose(const Temporal *temp1, const Temporal *temp2);
extern TInstant *nai_tpose_geo(const Temporal *temp, const GSERIALIZED *gs);
extern TInstant *nai_tpose_pose(const Temporal *temp, const Pose *pose);
extern TInstant *nai_tpose_tpose(const Temporal *temp1, const Temporal *temp2);
extern GSERIALIZED *shortestline_tpose_geo(const Temporal *temp, const GSERIALIZED *gs);
extern GSERIALIZED *shortestline_tpose_pose(const Temporal *temp, const Pose *pose);
extern GSERIALIZED *shortestline_tpose_tpose(const Temporal *temp1, const Temporal *temp2);

/*****************************************************************************
 * Spatial functions
 *****************************************************************************/

extern Set *tpose_points(const Temporal *temp);
extern Temporal *tpose_rotation(const Temporal *temp);
// extern Temporal *tpose_orientation(const Temporal *temp);
// extern GSERIALIZED *tpose_trajectory(const Temporal *temp);

/*****************************************************************************/

/* Spatial transformation functions for temporal points */

extern Temporal *tpose_tpoint(const Temporal *temp);
// extern TInstant *tposeinst_tgeompointinst(const TInstant *inst);

/*****************************************************************************/

/* Ever/always and temporal comparison functions */

extern int ever_eq_pose_tpose(const Pose *pose, const Temporal *temp);
extern int ever_eq_tpose_pose(const Temporal *temp, const Pose *pose);
extern int ever_ne_pose_tpose(const Pose *pose, const Temporal *temp);
extern int ever_ne_tpose_pose(const Temporal *temp, const Pose *pose);
extern int always_eq_pose_tpose(const Pose *pose, const Temporal *temp);
extern int always_eq_tpose_pose(const Temporal *temp, const Pose *pose);
extern int always_ne_pose_tpose(const Pose *pose, const Temporal *temp);
extern int always_ne_tpose_pose(const Temporal *temp, const Pose *pose);
extern int ever_eq_tpose_tpose(const Temporal *temp1, const Temporal *temp2);
extern int ever_ne_tpose_tpose(const Temporal *temp1, const Temporal *temp2);
extern int always_eq_tpose_tpose(const Temporal *temp1, const Temporal *temp2);
extern int always_ne_tpose_tpose(const Temporal *temp1, const Temporal *temp2);

extern Temporal *teq_pose_tpose(const Pose *pose, const Temporal *temp);
extern Temporal *tne_pose_tpose(const Pose *pose, const Temporal *temp);
extern Temporal *teq_tpose_pose(const Temporal *temp, const Pose *pose);
extern Temporal *tne_tpose_pose(const Temporal *temp, const Pose *pose);

/*****************************************************************************/

/* Ever and always spatial relationship functions */


/*****************************************************************************/

/* Temporal spatial relationship functions */


/*****************************************************************************/

#endif /* __MEOS_POSE_H__ */
