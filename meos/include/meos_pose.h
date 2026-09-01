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
 * @brief External API of the Mobility Engine Open Source (MEOS) library
 */

#ifndef __MEOS_POSE_H__
#define __MEOS_POSE_H__

/* C */
#include <stdbool.h>
#include <stdint.h>
/* MEOS */
#include <meos.h>
#include <meos_internal_geo.h>

/*****************************************************************************
 * Definitions
 *****************************************************************************/

/**
 * Opaque structure to represent pose values
 */
typedef struct Pose Pose;

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
      assert((set)->settype == T_POSESET); \
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

/* Input and output functions */

extern char *pose_as_ewkt(const Pose *pose, int maxdd);
extern char *pose_as_hexwkb(const Pose *pose, uint8_t variant, size_t *size_out);
extern char *pose_as_text(const Pose *pose, int maxdd);
extern uint8_t *pose_as_wkb(const Pose *pose, uint8_t variant, size_t *size_out);
extern Pose *pose_from_wkb(const uint8_t *wkb, size_t size);
extern Pose *pose_from_hexwkb(const char *hexwkb);
extern Pose *pose_in(const char *str);
extern char *pose_out(const Pose *pose, int maxdd);

/* OGC GeoPose JSON I/O — Basic-YPR + Basic-Quaternion conformance */

extern Pose *pose_from_geopose(const char *json);
extern char *pose_as_geopose(const Pose *pose, int conformance, int precision);
extern Temporal *tpose_from_geopose(const char *json);
extern char *tpose_as_geopose(const Temporal *temp, int conformance, int precision);
extern char *tpose_as_geopose_stream_header(const Temporal *temp, int precision);
extern char *tpose_as_geopose_stream_element(const Temporal *temp, const TInstant *inst, int precision);
extern char *tpose_as_geopose_stream(const Temporal *temp, int precision);

/**
 * @brief One frame of the OGC GeoPose registry, as the standard names it
 * @details A frame whose position depends on a runtime anchor states no
 * spatial reference system, and carries @p srid 0 for it.
 */
typedef struct
{
  int32_t frame_id;       /**< Stable key the registry states the frame under */
  const char *authority;  /**< Naming authority: EPSG, OGC or /geopose/1.0 */
  const char *code;       /**< Identifier the authority names the frame by */
  const char *name;       /**< Human-readable frame name */
  int32_t srid;           /**< Spatial reference system, 0 where parametric */
  bool is_geographic;     /**< True for a lat/lon/h frame */
  const char *description;/**< Free-form description for human readers */
} GeoPoseFrame;

extern const GeoPoseFrame *geopose_frames(int *count);
extern const GeoPoseFrame *geopose_frame(int32_t frame_id);
extern GSERIALIZED *pose_apply_geo(const Pose *pose, const GSERIALIZED *body);
extern Temporal *tpose_apply_geo(const Temporal *temp, const GSERIALIZED *body);
extern Temporal *tpose_compose_pose(const Temporal *body, const Pose *frame);
extern Temporal *pose_compose_tpose(const Pose *body, const Temporal *frame);
extern Temporal *tpose_compose_tpose(const Temporal *body, const Temporal *frame);
extern Temporal *tpose_inverse(const Temporal *temp);

/* Constructor functions */

extern Pose *pose_copy(const Pose *pose);
extern Pose *pose_make_2d(double x, double y, double theta, bool geodetic, int32_t srid);
extern Pose *pose_make_3d(double x, double y, double z, double W, double X, double Y, double Z, bool geodetic, int32_t srid);
extern Pose *pose_make_point2d(const GSERIALIZED *gs, double theta);
extern Pose *pose_make_point3d(const GSERIALIZED *gs, double W, double X, double Y, double Z);
extern Pose *pose_make_point3d_ypr(const GSERIALIZED *gs, double yaw, double pitch, double roll);

/* Conversion functions */

extern GSERIALIZED *pose_to_point(const Pose *pose);
extern STBox *pose_to_stbox(const Pose *pose);

/* Accessor functions */

extern uint32 pose_hash(const Pose *pose);
extern uint64 pose_hash_extended(const Pose *pose, uint64 seed);
extern double *pose_quaternion(const Pose *pose, int *count);
extern double *pose_ypr(const Pose *pose, int *count);
extern double pose_yaw(const Pose *pose);
extern double pose_pitch(const Pose *pose);
extern double pose_roll(const Pose *pose);
extern double pose_angular_distance(const Pose *pose1, const Pose *pose2);

/* Transformation functions */

extern Pose *pose_compose(const Pose *body, const Pose *frame);
extern Pose *pose_inverse(const Pose *pose);
extern Pose *pose_normalize(const Pose *pose);
extern Pose *pose_round(const Pose *pose, int maxdd);
extern Pose **posearr_round(const Pose **posearr, int count, int maxdd);

/* Spatial reference system functions */

extern Pose *pose_set_srid(const Pose *pose, int32_t srid);
extern int32_t pose_srid(const Pose *pose);
extern Pose *pose_transform(const Pose *pose, int32_t srid);
extern Pose *pose_transform_pipeline(const Pose *pose, const char *pipelinestr, int32_t srid, bool is_forward);

/* Bounding box functions */

extern STBox *pose_tstzspan_to_stbox(const Pose *pose, const Span *s);
extern STBox *pose_timestamptz_to_stbox(const Pose *pose, TimestampTz t);

/* Distance functions */

extern double distance_pose_geo(const Pose *pose, const GSERIALIZED *gs);
extern double distance_pose_pose(const Pose *pose1, const Pose *pose2);
extern double distance_pose_stbox(const Pose *pose, const STBox *box);

/* Comparison functions */

extern int pose_cmp(const Pose *pose1, const Pose *pose2);
extern bool pose_eq(const Pose *pose1, const Pose *pose2);
extern bool pose_ge(const Pose *pose1, const Pose *pose2);
extern bool pose_gt(const Pose *pose1, const Pose *pose2);
extern bool pose_le(const Pose *pose1, const Pose *pose2);
extern bool pose_lt(const Pose *pose1, const Pose *pose2);
extern bool pose_ne(const Pose *pose1, const Pose *pose2);
extern bool pose_nsame(const Pose *pose1, const Pose *pose2);
extern bool pose_same(const Pose *pose1, const Pose *pose2);

/******************************************************************************
 * Functions for pose sets
 ******************************************************************************/

/* Input and output functions */

extern Set *poseset_in(const char *str);
extern char *poseset_out(const Set *s, int maxdd);

/* Constructor functions */

extern Set *poseset_make(const Pose **values, int count);

/* Conversion functions */

extern Set *pose_to_set(const Pose *pose);

/* Accessor functions */

extern Pose *poseset_end_value(const Set *s);
extern Pose *poseset_start_value(const Set *s);
extern bool poseset_value_n(const Set *s, int n, Pose **result);
extern Pose **poseset_values(const Set *s, int *count);

/* Set operations */

extern bool contained_pose_set(const Pose *pose, const Set *s);
extern bool contains_set_pose(const Set *s, Pose *pose);
extern Set *intersection_pose_set(const Pose *pose, const Set *s);
extern Set *intersection_set_pose(const Set *s, const Pose *pose);
extern Set *minus_pose_set(const Pose *pose, const Set *s);
extern Set *minus_set_pose(const Set *s, const Pose *pose);
extern Set *pose_union_transfn(Set *state, const Pose *pose);
extern Set *union_pose_set(const Pose *pose, const Set *s);
extern Set *union_set_pose(const Set *s, const Pose *pose);

/*===========================================================================*
 * Functions for temporal poses
 *===========================================================================*/

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

extern Temporal *tpose_from_mfjson(const char *str);
extern Temporal *tpose_in(const char *str);

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

extern TInstant *tposeinst_make(const Pose *pose, TimestampTz t);
extern Temporal *tpose_from_base_temp(const Pose *pose, const Temporal *temp);
extern TSequence *tposeseq_from_base_tstzset(const Pose *pose, const Set *s);
extern TSequence *tposeseq_from_base_tstzspan(const Pose *pose, const Span *s, interpType interp);
extern TSequenceSet *tposeseqset_from_base_tstzspanset(const Pose *pose, const SpanSet *ss, interpType interp);

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

extern Temporal *tpose_make(const Temporal *tpoint, const Temporal *ttheta);
extern Temporal *tpose_to_tpoint(const Temporal *temp);

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

extern Pose *tpose_end_value(const Temporal *temp);
extern Set *tpose_points(const Temporal *temp);
// extern Temporal *tpose_quaternion(const Temporal *temp);
extern Temporal *tpose_yaw(const Temporal *temp);
extern Temporal *tpose_pitch(const Temporal *temp);
extern Temporal *tpose_roll(const Temporal *temp);
extern Temporal *tpose_speed(const Temporal *temp);
extern Temporal *tpose_angular_speed(const Temporal *temp);
extern Pose *tpose_start_value(const Temporal *temp);
extern GSERIALIZED *tpose_trajectory(const Temporal *temp);
extern bool tpose_value_at_timestamptz(const Temporal *temp, TimestampTz t, bool strict, Pose **result);
extern bool tpose_value_n(const Temporal *temp, int n, Pose **result);
extern Pose **tpose_values(const Temporal *temp, int *count);

/*****************************************************************************
 * Restriction functions
 *****************************************************************************/

extern Temporal *tpose_at_elevation(const Temporal *temp, const Span *s);
extern Temporal *tpose_at_geom(const Temporal *temp, const GSERIALIZED *gs);
extern Temporal *tpose_at_stbox(const Temporal *temp, const STBox *box, bool border_inc);
extern Temporal *tpose_at_pose(const Temporal *temp, const Pose *pose);
extern Temporal *tpose_minus_elevation(const Temporal *temp, const Span *s);
extern Temporal *tpose_minus_geom(const Temporal *temp, const GSERIALIZED *gs);
extern Temporal *tpose_minus_pose(const Temporal *temp, const Pose *pose);
extern Temporal *tpose_minus_stbox(const Temporal *temp, const STBox *box, bool border_inc);

/*****************************************************************************
 * Distance functions
 *****************************************************************************/

extern Temporal *tdistance_tpose_pose(const Temporal *temp, const Pose *pose);
extern Temporal *tdistance_tpose_geo(const Temporal *temp, const GSERIALIZED *gs);
extern Temporal *tdistance_tpose_tpose(const Temporal *temp1, const Temporal *temp2);
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
 * Comparison functions
 *****************************************************************************/

/* Ever/always and temporal comparison functions */

extern int always_eq_pose_tpose(const Pose *pose, const Temporal *temp);
extern int always_eq_tpose_pose(const Temporal *temp, const Pose *pose);
extern int always_eq_tpose_tpose(const Temporal *temp1, const Temporal *temp2);
extern int always_ne_pose_tpose(const Pose *pose, const Temporal *temp);
extern int always_ne_tpose_pose(const Temporal *temp, const Pose *pose);
extern int always_ne_tpose_tpose(const Temporal *temp1, const Temporal *temp2);
extern int ever_eq_pose_tpose(const Pose *pose, const Temporal *temp);
extern int ever_eq_tpose_pose(const Temporal *temp, const Pose *pose);
extern int ever_eq_tpose_tpose(const Temporal *temp1, const Temporal *temp2);
extern int ever_ne_pose_tpose(const Pose *pose, const Temporal *temp);
extern int ever_ne_tpose_pose(const Temporal *temp, const Pose *pose);
extern int ever_ne_tpose_tpose(const Temporal *temp1, const Temporal *temp2);

/*****************************************************************************/

extern Temporal *teq_pose_tpose(const Pose *pose, const Temporal *temp);
extern Temporal *teq_tpose_pose(const Temporal *temp, const Pose *pose);
extern Temporal *tne_pose_tpose(const Pose *pose, const Temporal *temp);
extern Temporal *tne_tpose_pose(const Temporal *temp, const Pose *pose);

/*****************************************************************************/

/* Ever and always spatial relationship functions */


/*****************************************************************************/

/* Spatiotemporal relationship functions */


/*****************************************************************************/

/*****************************************************************************
 * Definitions
 *****************************************************************************/

/**
 * Opaque structure to represent pose chain values
 */
typedef struct PoseChain PoseChain;

/*****************************************************************************
 * Validity macros
 *****************************************************************************/

/**
 * @brief Macro for ensuring that the set passed as argument is a pose chain
 * set
 */
#if MEOS
  #define VALIDATE_POSECHAINSET(set, ret) \
    do { \
          if (! ensure_not_null((void *) set) || \
              ! ensure_set_isof_type((set), T_POSECHAINSET) ) \
           return (ret); \
    } while (0)
#else
  #define VALIDATE_POSECHAINSET(set, ret) \
    do { \
      assert(set); \
      assert((set)->settype == T_POSECHAINSET); \
    } while (0)
#endif

/**
 * @brief Macro for ensuring that a temporal value is a temporal pose chain
 * @note The macro works for the Temporal type and its subtypes TInstant,
 * TSequence, and TSequenceSet
 */
#if MEOS
  #define VALIDATE_TPOSECHAIN(temp, ret) \
    do { \
          if (! ensure_not_null((void *) (temp)) || \
              ! ensure_temporal_isof_type((Temporal *) (temp), \
                T_TPOSECHAIN) ) \
           return (ret); \
    } while (0)
#else
  #define VALIDATE_TPOSECHAIN(temp, ret) \
    do { \
      assert(temp); \
      assert(((Temporal *) (temp))->temptype == T_TPOSECHAIN); \
    } while (0)
#endif

/******************************************************************************
 * Functions for pose chains
 ******************************************************************************/

/* Input and output functions */

extern PoseChain *posechain_in(const char *str);
extern char *posechain_out(const PoseChain *pc, int maxdd);
extern char *posechain_as_text(const PoseChain *pc, int maxdd);
extern char *posechain_as_ewkt(const PoseChain *pc, int maxdd);
extern uint8_t *posechain_as_wkb(const PoseChain *pc, uint8_t variant, size_t *size_out);
extern char *posechain_as_hexwkb(const PoseChain *pc, uint8_t variant, size_t *size_out);
extern PoseChain *posechain_from_wkb(const uint8_t *wkb, size_t size);
extern PoseChain *posechain_from_hexwkb(const char *hexwkb);

/* Constructor functions */

extern PoseChain *posechain_make(const Pose **poses, int count);
extern PoseChain *posechain_copy(const PoseChain *pc);
extern PoseChain *posechain_append(const PoseChain *pc, const Pose *pose);

/* Conversion functions */

extern PoseChain *pose_to_posechain(const Pose *pose);
extern Pose *posechain_to_pose(const PoseChain *pc);
extern Pose *posechain_prefix_pose(const PoseChain *pc, int n);
extern GSERIALIZED *posechain_to_point(const PoseChain *pc);
extern STBox *posechain_to_stbox(const PoseChain *pc);
extern STBox *posechain_timestamptz_to_stbox(const PoseChain *pc, TimestampTz t);
extern STBox *posechain_tstzspan_to_stbox(const PoseChain *pc, const Span *s);

/* Accessor functions */

extern int posechain_num_poses(const PoseChain *pc);
extern Pose *posechain_start_pose(const PoseChain *pc);
extern Pose *posechain_end_pose(const PoseChain *pc);
extern Pose *posechain_pose_n(const PoseChain *pc, int n);
extern Pose **posechain_poses(const PoseChain *pc, int *count);
extern uint32 posechain_hash(const PoseChain *pc);
extern uint64 posechain_hash_extended(const PoseChain *pc, uint64 seed);

/* Transformation functions */

extern PoseChain *posechain_round(const PoseChain *pc, int maxdd);

/* Spatial reference system functions */

extern int32_t posechain_srid(const PoseChain *pc);
extern PoseChain *posechain_set_srid(const PoseChain *pc, int32_t srid);
extern PoseChain *posechain_transform(const PoseChain *pc, int32_t srid_to);
extern PoseChain *posechain_transform_pipeline(const PoseChain *pc, const char *pipeline, int32_t srid_to, bool is_forward);

/*****************************************************************************
 * Functions for temporal pose chains
 *****************************************************************************/

/* OGC GeoPose input/output functions for temporal pose chains */

extern Temporal *tposechain_from_geopose(const char *json);
extern char *tposechain_as_geopose(const Temporal *temp, int precision);
extern char *tposechainarr_as_geopose(const Temporal **temparr, int count, int precision);

/* Comparison functions */

extern bool posechain_eq(const PoseChain *pc1, const PoseChain *pc2);
extern bool posechain_ne(const PoseChain *pc1, const PoseChain *pc2);
extern bool posechain_same(const PoseChain *pc1, const PoseChain *pc2);
extern bool posechain_nsame(const PoseChain *pc1, const PoseChain *pc2);
extern int posechain_cmp(const PoseChain *pc1, const PoseChain *pc2);
extern bool posechain_lt(const PoseChain *pc1, const PoseChain *pc2);
extern bool posechain_le(const PoseChain *pc1, const PoseChain *pc2);
extern bool posechain_gt(const PoseChain *pc1, const PoseChain *pc2);
extern bool posechain_ge(const PoseChain *pc1, const PoseChain *pc2);

/******************************************************************************
 * Functions for pose chain sets
 ******************************************************************************/

/* Input and output functions */

extern Set *posechainset_in(const char *str);
extern char *posechainset_out(const Set *s, int maxdd);

/* Constructor functions */

extern Set *posechainset_make(const PoseChain **values, int count);

/* Conversion functions */

extern Set *posechain_to_set(const PoseChain *pc);

/* Accessor functions */

extern PoseChain *posechainset_end_value(const Set *s);
extern PoseChain *posechainset_start_value(const Set *s);
extern bool posechainset_value_n(const Set *s, int n, PoseChain **result);
extern PoseChain **posechainset_values(const Set *s, int *count);

/* Set operations */

extern bool contained_posechain_set(const PoseChain *pc, const Set *s);
extern bool contains_set_posechain(const Set *s, PoseChain *pc);
extern Set *intersection_posechain_set(const PoseChain *pc, const Set *s);
extern Set *intersection_set_posechain(const Set *s, const PoseChain *pc);
extern Set *minus_posechain_set(const PoseChain *pc, const Set *s);
extern Set *minus_set_posechain(const Set *s, const PoseChain *pc);
extern Set *posechain_union_transfn(Set *state, const PoseChain *pc);
extern Set *union_posechain_set(const PoseChain *pc, const Set *s);
extern Set *union_set_posechain(const Set *s, const PoseChain *pc);

/******************************************************************************
 * Functions for temporal pose chains
 ******************************************************************************/

/* Input and output functions */

extern Temporal *tposechain_in(const char *str);
extern Temporal *tposechain_from_mfjson(const char *mfjson);

/* Constructor functions */

extern Temporal *tposechain_from_base_temp(const PoseChain *pc, const Temporal *temp);

/* Conversion functions */

extern Temporal *tposechain_to_tpose(const Temporal *temp);

/* Accessor functions */

extern int tposechain_num_poses(const Temporal *temp);

/* Ever/always and temporal comparison functions */

extern int always_eq_posechain_tposechain(const PoseChain *posechain, const Temporal *temp);
extern int always_eq_tposechain_posechain(const Temporal *temp, const PoseChain *posechain);
extern int always_eq_tposechain_tposechain(const Temporal *temp1, const Temporal *temp2);
extern int always_ne_posechain_tposechain(const PoseChain *posechain, const Temporal *temp);
extern int always_ne_tposechain_posechain(const Temporal *temp, const PoseChain *posechain);
extern int always_ne_tposechain_tposechain(const Temporal *temp1, const Temporal *temp2);
extern int ever_eq_posechain_tposechain(const PoseChain *posechain, const Temporal *temp);
extern int ever_eq_tposechain_posechain(const Temporal *temp, const PoseChain *posechain);
extern int ever_eq_tposechain_tposechain(const Temporal *temp1, const Temporal *temp2);
extern int ever_ne_posechain_tposechain(const PoseChain *posechain, const Temporal *temp);
extern int ever_ne_tposechain_posechain(const Temporal *temp, const PoseChain *posechain);
extern int ever_ne_tposechain_tposechain(const Temporal *temp1, const Temporal *temp2);

/*****************************************************************************/

extern Temporal *teq_posechain_tposechain(const PoseChain *posechain, const Temporal *temp);
extern Temporal *teq_tposechain_posechain(const Temporal *temp, const PoseChain *posechain);
extern Temporal *tne_posechain_tposechain(const PoseChain *posechain, const Temporal *temp);
extern Temporal *tne_tposechain_posechain(const Temporal *temp, const PoseChain *posechain);

/*****************************************************************************/

#endif /* __MEOS_POSE_H__ */
