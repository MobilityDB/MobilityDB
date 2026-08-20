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


/*****************************************************************************/

#endif /* __MEOS_POSECHAIN_H__ */
