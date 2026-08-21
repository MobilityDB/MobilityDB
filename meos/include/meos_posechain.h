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

#ifndef __MEOS_POSECHAIN_H__
#define __MEOS_POSECHAIN_H__

/* C */
#include <stdbool.h>
#include <stdint.h>
/* MEOS */
#include <meos.h>
#include <meos_internal_geo.h>
#include <meos_pose.h>

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

extern int tposechain_num_links(const Temporal *temp);

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

#endif /* __MEOS_POSECHAIN_H__ */
